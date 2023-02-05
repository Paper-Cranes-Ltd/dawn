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

////////////////////////////////////////////////////////////////////////////////
// File generated by tools/src/cmd/gen
// using the template:
//   src/tint/ast/diagnostic_control.cc.tmpl
//
// Do not modify this file directly
////////////////////////////////////////////////////////////////////////////////

#include "src/tint/ast/diagnostic_control.h"

#include <string>

#include "src/tint/ast/identifier.h"
#include "src/tint/ast/templated_identifier.h"

namespace tint::ast {

DiagnosticControl::DiagnosticControl(DiagnosticSeverity sev, const Identifier* rule)
    : severity(sev), rule_name(rule) {
    TINT_ASSERT(AST, rule != nullptr);
    if (rule) {
        // It is invalid for a diagnostic rule name to be templated
        TINT_ASSERT(AST, !rule->Is<TemplatedIdentifier>());
    }
}

diag::Severity ToSeverity(DiagnosticSeverity sc) {
    switch (sc) {
        case DiagnosticSeverity::kError:
            return diag::Severity::Error;
        case DiagnosticSeverity::kWarning:
            return diag::Severity::Warning;
        case DiagnosticSeverity::kInfo:
            return diag::Severity::Note;
        default:
            return diag::Severity::InternalCompilerError;
    }
}

/// ParseDiagnosticSeverity parses a DiagnosticSeverity from a string.
/// @param str the string to parse
/// @returns the parsed enum, or DiagnosticSeverity::kUndefined if the string could not be parsed.
DiagnosticSeverity ParseDiagnosticSeverity(std::string_view str) {
    if (str == "error") {
        return DiagnosticSeverity::kError;
    }
    if (str == "info") {
        return DiagnosticSeverity::kInfo;
    }
    if (str == "off") {
        return DiagnosticSeverity::kOff;
    }
    if (str == "warning") {
        return DiagnosticSeverity::kWarning;
    }
    return DiagnosticSeverity::kUndefined;
}

std::ostream& operator<<(std::ostream& out, DiagnosticSeverity value) {
    switch (value) {
        case DiagnosticSeverity::kUndefined:
            return out << "undefined";
        case DiagnosticSeverity::kError:
            return out << "error";
        case DiagnosticSeverity::kInfo:
            return out << "info";
        case DiagnosticSeverity::kOff:
            return out << "off";
        case DiagnosticSeverity::kWarning:
            return out << "warning";
    }
    return out << "<unknown>";
}

/// ParseDiagnosticRule parses a DiagnosticRule from a string.
/// @param str the string to parse
/// @returns the parsed enum, or DiagnosticRule::kUndefined if the string could not be parsed.
DiagnosticRule ParseDiagnosticRule(std::string_view str) {
    if (str == "chromium_unreachable_code") {
        return DiagnosticRule::kChromiumUnreachableCode;
    }
    if (str == "derivative_uniformity") {
        return DiagnosticRule::kDerivativeUniformity;
    }
    return DiagnosticRule::kUndefined;
}

std::ostream& operator<<(std::ostream& out, DiagnosticRule value) {
    switch (value) {
        case DiagnosticRule::kUndefined:
            return out << "undefined";
        case DiagnosticRule::kChromiumUnreachableCode:
            return out << "chromium_unreachable_code";
        case DiagnosticRule::kDerivativeUniformity:
            return out << "derivative_uniformity";
    }
    return out << "<unknown>";
}

}  // namespace tint::ast