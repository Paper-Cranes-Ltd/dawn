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
//   src/tint/builtin/diagnostic_rule.h.tmpl
//
// Do not modify this file directly
////////////////////////////////////////////////////////////////////////////////

#ifndef SRC_TINT_BUILTIN_DIAGNOSTIC_RULE_H_
#define SRC_TINT_BUILTIN_DIAGNOSTIC_RULE_H_

#include <string>

namespace tint::builtin {

/// The diagnostic rule.
enum class DiagnosticRule {
    kUndefined,
    kChromiumUnreachableCode,
    kDerivativeUniformity,
};

/// @param out the std::ostream to write to
/// @param value the DiagnosticRule
/// @returns `out` so calls can be chained
std::ostream& operator<<(std::ostream& out, DiagnosticRule value);

/// ParseDiagnosticRule parses a DiagnosticRule from a string.
/// @param str the string to parse
/// @returns the parsed enum, or DiagnosticRule::kUndefined if the string could not be parsed.
DiagnosticRule ParseDiagnosticRule(std::string_view str);

constexpr const char* kDiagnosticRuleStrings[] = {
    "chromium_unreachable_code",
    "derivative_uniformity",
};

}  // namespace tint::builtin

#endif  // SRC_TINT_BUILTIN_DIAGNOSTIC_RULE_H_