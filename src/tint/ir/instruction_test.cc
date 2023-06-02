// Copyright 2023 The Tint Authors.
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

#include "gtest/gtest-spi.h"
#include "src/tint/ir/block.h"
#include "src/tint/ir/builder.h"
#include "src/tint/ir/ir_test_helper.h"
#include "src/tint/ir/module.h"

namespace tint::ir {
namespace {

using IR_InstructionTest = IRTestHelper;

TEST_F(IR_InstructionTest, InsertBefore) {
    auto* inst1 = b.CreateLoop();
    auto* inst2 = b.CreateLoop();
    auto* blk = b.CreateBlock();
    blk->Append(inst2);
    inst1->InsertBefore(inst2);
    EXPECT_EQ(2u, blk->Length());
    EXPECT_EQ(inst1->Block(), blk);
}

TEST_F(IR_InstructionTest, Fail_InsertBeforeNullptr) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};

            auto* inst1 = b.CreateLoop();
            inst1->InsertBefore(nullptr);
        },
        "");
}

TEST_F(IR_InstructionTest, Fail_InsertBeforeNotInserted) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};

            auto* inst1 = b.CreateLoop();
            auto* inst2 = b.CreateLoop();
            inst1->InsertBefore(inst2);
        },
        "");
}

TEST_F(IR_InstructionTest, InsertAfter) {
    auto* inst1 = b.CreateLoop();
    auto* inst2 = b.CreateLoop();
    auto* blk = b.CreateBlock();
    blk->Append(inst2);
    inst1->InsertAfter(inst2);
    EXPECT_EQ(2u, blk->Length());
    EXPECT_EQ(inst1->Block(), blk);
}

TEST_F(IR_InstructionTest, Fail_InsertAfterNullptr) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};

            auto* inst1 = b.CreateLoop();
            inst1->InsertAfter(nullptr);
        },
        "");
}

TEST_F(IR_InstructionTest, Fail_InsertAfterNotInserted) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};

            auto* inst1 = b.CreateLoop();
            auto* inst2 = b.CreateLoop();
            inst1->InsertAfter(inst2);
        },
        "");
}

TEST_F(IR_InstructionTest, Replace) {
    auto* inst1 = b.CreateLoop();
    auto* inst2 = b.CreateLoop();
    auto* blk = b.CreateBlock();
    blk->Append(inst2);
    inst2->Replace(inst1);
    EXPECT_EQ(1u, blk->Length());
    EXPECT_EQ(inst1->Block(), blk);
    EXPECT_EQ(inst2->Block(), nullptr);
}

TEST_F(IR_InstructionTest, Fail_ReplaceNullptr) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};

            auto* inst1 = b.CreateLoop();
            auto* blk = b.CreateBlock();
            blk->Append(inst1);
            inst1->Replace(nullptr);
        },
        "");
}

TEST_F(IR_InstructionTest, Fail_ReplaceNotInserted) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};

            auto* inst1 = b.CreateLoop();
            auto* inst2 = b.CreateLoop();
            inst1->Replace(inst2);
        },
        "");
}

TEST_F(IR_InstructionTest, Remove) {
    auto* inst1 = b.CreateLoop();
    auto* blk = b.CreateBlock();
    blk->Append(inst1);
    EXPECT_EQ(1u, blk->Length());

    inst1->Remove();
    EXPECT_EQ(0u, blk->Length());
    EXPECT_EQ(inst1->Block(), nullptr);
}

TEST_F(IR_InstructionTest, Fail_RemoveNotInserted) {
    EXPECT_FATAL_FAILURE(
        {
            Module mod;
            Builder b{mod};

            auto* inst1 = b.CreateLoop();
            inst1->Remove();
        },
        "");
}

}  // namespace
}  // namespace tint::ir
