// Copyright 2022 The Tint Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "src/tint/transform/spirv_atomic.h"

#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "src/tint/program_builder.h"
#include "src/tint/sem/block_statement.h"
#include "src/tint/sem/function.h"
#include "src/tint/sem/index_accessor_expression.h"
#include "src/tint/sem/member_accessor_expression.h"
#include "src/tint/sem/statement.h"
#include "src/tint/type/reference.h"
#include "src/tint/utils/map.h"
#include "src/tint/utils/unique_vector.h"

TINT_INSTANTIATE_TYPEINFO(tint::transform::SpirvAtomic);
TINT_INSTANTIATE_TYPEINFO(tint::transform::SpirvAtomic::Stub);

namespace tint::transform {

using namespace tint::number_suffixes;  // NOLINT

/// PIMPL state for the transform
struct SpirvAtomic::State {
  private:
    /// A struct that has been forked because a subset of members were made atomic.
    struct ForkedStruct {
        Symbol name;
        std::unordered_set<size_t> atomic_members;
    };

    /// The source program
    const Program* const src;
    /// The target program builder
    ProgramBuilder b;
    /// The clone context
    CloneContext ctx = {&b, src, /* auto_clone_symbols */ true};
    std::unordered_map<const ast::Struct*, ForkedStruct> forked_structs;
    std::unordered_set<const sem::Variable*> atomic_variables;
    utils::UniqueVector<const sem::Expression*, 8> atomic_expressions;

  public:
    /// Constructor
    /// @param program the source program
    explicit State(const Program* program) : src(program) {}

    /// Runs the transform
    /// @returns the new program or SkipTransform if the transform is not required
    ApplyResult Run() {
        bool made_changes = false;

        // Look for stub functions generated by the SPIR-V reader, which are used as placeholders
        // for atomic builtin calls.
        for (auto* fn : ctx.src->AST().Functions()) {
            if (auto* stub = ast::GetAttribute<Stub>(fn->attributes)) {
                auto* sem = ctx.src->Sem().Get(fn);

                for (auto* call : sem->CallSites()) {
                    // The first argument is always the atomic.
                    // The stub passes this by value, whereas the builtin wants a pointer.
                    // Take the address of the atomic argument.
                    auto& args = call->Declaration()->args;
                    auto out_args = ctx.Clone(args);
                    out_args[0] = b.AddressOf(out_args[0]);

                    // Replace all callsites of this stub to a call to the real builtin
                    if (stub->builtin == sem::BuiltinType::kAtomicCompareExchangeWeak) {
                        // atomicCompareExchangeWeak returns a struct, so insert a call to it above
                        // the current statement, and replace the current call with the struct's
                        // `old_value` member.
                        auto* block = call->Stmt()->Block()->Declaration();
                        auto old_value = b.Symbols().New("old_value");
                        auto old_value_decl = b.Decl(b.Let(
                            old_value,
                            b.MemberAccessor(b.Call(sem::str(stub->builtin), std::move(out_args)),
                                             b.Expr("old_value"))));
                        ctx.InsertBefore(block->statements, call->Stmt()->Declaration(),
                                         old_value_decl);
                        ctx.Replace(call->Declaration(), b.Expr(old_value));
                    } else {
                        ctx.Replace(call->Declaration(),
                                    b.Call(sem::str(stub->builtin), std::move(out_args)));
                    }

                    // Keep track of this expression. We'll need to modify the root identifier /
                    // structure to be atomic.
                    atomic_expressions.Add(ctx.src->Sem().Get(args[0]));
                }

                // Remove the stub from the output program
                ctx.Remove(ctx.src->AST().GlobalDeclarations(), fn);
                made_changes = true;
            }
        }

        if (!made_changes) {
            return SkipTransform;
        }

        // Transform all variables and structure members that were used in atomic operations as
        // atomic types. This propagates up originating expression chains.
        ProcessAtomicExpressions();

        // If we need to change structure members, then fork them.
        if (!forked_structs.empty()) {
            ctx.ReplaceAll([&](const ast::Struct* str) {
                // Always emit the original structure. This allows unrelated usage of the structure
                // to continue working.
                // auto* original = ctx.CloneWithoutTransform(str);

                // Is `str` a structure we need to fork?
                if (auto it = forked_structs.find(str); it != forked_structs.end()) {
                    const auto& forked = it->second;

                    // Re-create the structure swapping in the atomic-flavoured members
                    utils::Vector<const ast::StructMember*, 8> members;
                    members.Reserve(str->members.Length());
                    for (size_t i = 0; i < str->members.Length(); i++) {
                        auto* member = str->members[i];
                        if (forked.atomic_members.count(i)) {
                            auto* type = AtomicTypeFor(ctx.src->Sem().Get(member)->Type());
                            auto name = ctx.src->Symbols().NameFor(member->symbol);
                            members.Push(b.Member(name, type, ctx.Clone(member->attributes)));
                        } else {
                            members.Push(ctx.Clone(member));
                        }
                    }
                    b.Structure(forked.name, std::move(members));
                }

                // return original;
                return nullptr;
            });
        }

        // Replace assignments and decls from atomic variables with atomicLoads, and assignments to
        // atomic variables with atomicStores.
        ReplaceLoadsAndStores();

        ctx.Clone();
        return Program(std::move(b));
    }

  private:
    ForkedStruct& Fork(const ast::Struct* str) {
        auto& forked = forked_structs[str];
        if (!forked.name.IsValid()) {
            forked.name = b.Symbols().New(ctx.src->Symbols().NameFor(str->name) + "_atomic");
        }
        return forked;
    }

    void ProcessAtomicExpressions() {
        for (size_t i = 0; i < atomic_expressions.Length(); i++) {
            Switch(
                atomic_expressions[i],  //
                [&](const sem::VariableUser* user) {
                    auto* v = user->Variable()->Declaration();
                    if (v->type && atomic_variables.emplace(user->Variable()).second) {
                        ctx.Replace(v->type, AtomicTypeFor(user->Variable()->Type()));
                    }
                    if (auto* ctor = user->Variable()->Initializer()) {
                        atomic_expressions.Add(ctor);
                    }
                },
                [&](const sem::StructMemberAccess* access) {
                    // Fork the struct (the first time) and mark member(s) that need to be made
                    // atomic.
                    auto* member = access->Member();
                    Fork(member->Struct()->Declaration()).atomic_members.emplace(member->Index());
                    atomic_expressions.Add(access->Object());
                },
                [&](const sem::IndexAccessorExpression* index) {
                    atomic_expressions.Add(index->Object());
                },
                [&](const sem::Expression* e) {
                    if (auto* unary = e->Declaration()->As<ast::UnaryOpExpression>()) {
                        atomic_expressions.Add(ctx.src->Sem().Get(unary->expr));
                    }
                });
        }
    }

    const ast::Type* AtomicTypeFor(const type::Type* ty) {
        return Switch(
            ty,  //
            [&](const type::I32*) { return b.ty.atomic(CreateASTTypeFor(ctx, ty)); },
            [&](const type::U32*) { return b.ty.atomic(CreateASTTypeFor(ctx, ty)); },
            [&](const sem::Struct* str) { return b.ty.type_name(Fork(str->Declaration()).name); },
            [&](const sem::Array* arr) -> const ast::Type* {
                if (arr->Count()->Is<type::RuntimeArrayCount>()) {
                    return b.ty.array(AtomicTypeFor(arr->ElemType()));
                }
                auto count = arr->ConstantCount();
                if (!count) {
                    ctx.dst->Diagnostics().add_error(
                        diag::System::Transform,
                        "the SpirvAtomic transform does not currently support array counts that "
                        "use override values");
                    count = 1;
                }
                return b.ty.array(AtomicTypeFor(arr->ElemType()), u32(count.value()));
            },
            [&](const type::Pointer* ptr) {
                return b.ty.pointer(AtomicTypeFor(ptr->StoreType()), ptr->AddressSpace(),
                                    ptr->Access());
            },
            [&](const type::Reference* ref) { return AtomicTypeFor(ref->StoreType()); },
            [&](Default) {
                TINT_ICE(Transform, b.Diagnostics())
                    << "unhandled type: " << ty->FriendlyName(ctx.src->Symbols());
                return nullptr;
            });
    }

    void ReplaceLoadsAndStores() {
        // Returns true if 'e' is a reference to an atomic variable or struct member
        auto is_ref_to_atomic_var = [&](const sem::Expression* e) {
            if (tint::Is<type::Reference>(e->Type()) && e->RootIdentifier() &&
                (atomic_variables.count(e->RootIdentifier()) != 0)) {
                // If it's a struct member, make sure it's one we marked as atomic
                if (auto* ma = e->As<sem::StructMemberAccess>()) {
                    auto it = forked_structs.find(ma->Member()->Struct()->Declaration());
                    if (it != forked_structs.end()) {
                        auto& forked = it->second;
                        return forked.atomic_members.count(ma->Member()->Index()) != 0;
                    }
                }
                return true;
            }
            return false;
        };

        // Look for loads and stores via assignments and decls of atomic variables we've collected
        // so far, and replace them with atomicLoad and atomicStore.
        for (auto* atomic_var : atomic_variables) {
            for (auto* vu : atomic_var->Users()) {
                Switch(
                    vu->Stmt()->Declaration(),
                    [&](const ast::AssignmentStatement* assign) {
                        auto* sem_lhs = ctx.src->Sem().Get(assign->lhs);
                        if (is_ref_to_atomic_var(sem_lhs)) {
                            ctx.Replace(assign, [=] {
                                auto* lhs = ctx.CloneWithoutTransform(assign->lhs);
                                auto* rhs = ctx.CloneWithoutTransform(assign->rhs);
                                auto* call = b.Call(sem::str(sem::BuiltinType::kAtomicStore),
                                                    b.AddressOf(lhs), rhs);
                                return b.CallStmt(call);
                            });
                            return;
                        }

                        auto sem_rhs = ctx.src->Sem().Get(assign->rhs);
                        if (is_ref_to_atomic_var(sem_rhs)) {
                            ctx.Replace(assign->rhs, [=] {
                                auto* rhs = ctx.CloneWithoutTransform(assign->rhs);
                                return b.Call(sem::str(sem::BuiltinType::kAtomicLoad),
                                              b.AddressOf(rhs));
                            });
                            return;
                        }
                    },
                    [&](const ast::VariableDeclStatement* decl) {
                        auto* var = decl->variable;
                        if (auto* sem_init = ctx.src->Sem().Get(var->initializer)) {
                            if (is_ref_to_atomic_var(sem_init)) {
                                ctx.Replace(var->initializer, [=] {
                                    auto* rhs = ctx.CloneWithoutTransform(var->initializer);
                                    return b.Call(sem::str(sem::BuiltinType::kAtomicLoad),
                                                  b.AddressOf(rhs));
                                });
                                return;
                            }
                        }
                    });
            }
        }
    }
};

SpirvAtomic::SpirvAtomic() = default;
SpirvAtomic::~SpirvAtomic() = default;

SpirvAtomic::Stub::Stub(ProgramID pid, ast::NodeID nid, sem::BuiltinType b)
    : Base(pid, nid), builtin(b) {}
SpirvAtomic::Stub::~Stub() = default;
std::string SpirvAtomic::Stub::InternalName() const {
    return "@internal(spirv-atomic " + std::string(sem::str(builtin)) + ")";
}

const SpirvAtomic::Stub* SpirvAtomic::Stub::Clone(CloneContext* ctx) const {
    return ctx->dst->ASTNodes().Create<SpirvAtomic::Stub>(ctx->dst->ID(),
                                                          ctx->dst->AllocateNodeID(), builtin);
}

Transform::ApplyResult SpirvAtomic::Apply(const Program* src, const DataMap&, DataMap&) const {
    return State{src}.Run();
}

}  // namespace tint::transform
