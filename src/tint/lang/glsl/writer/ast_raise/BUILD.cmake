# Copyright 2023 The Tint Authors.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

################################################################################
# File generated by 'tools/src/cmd/gen' using the template:
#   tools/src/cmd/gen/build/BUILD.cmake.tmpl
#
# To regenerate run: './tools/run gen'
#
#                       Do not modify this file directly
################################################################################

if(TINT_BUILD_GLSL_WRITER)
################################################################################
# Target:    tint_lang_glsl_writer_ast_raise
# Kind:      lib
# Condition: TINT_BUILD_GLSL_WRITER
################################################################################
tint_add_target(tint_lang_glsl_writer_ast_raise lib
  lang/glsl/writer/ast_raise/combine_samplers.cc
  lang/glsl/writer/ast_raise/combine_samplers.h
  lang/glsl/writer/ast_raise/pad_structs.cc
  lang/glsl/writer/ast_raise/pad_structs.h
  lang/glsl/writer/ast_raise/texture_1d_to_2d.cc
  lang/glsl/writer/ast_raise/texture_1d_to_2d.h
  lang/glsl/writer/ast_raise/texture_builtins_from_uniform.cc
  lang/glsl/writer/ast_raise/texture_builtins_from_uniform.h
)

tint_target_add_dependencies(tint_lang_glsl_writer_ast_raise lib
  tint_api_common
  tint_api_options
  tint_lang_core
  tint_lang_core_constant
  tint_lang_core_type
  tint_lang_wgsl
  tint_lang_wgsl_ast
  tint_lang_wgsl_ast_transform
  tint_lang_wgsl_program
  tint_lang_wgsl_resolver
  tint_lang_wgsl_sem
  tint_utils_containers
  tint_utils_diagnostic
  tint_utils_ice
  tint_utils_id
  tint_utils_macros
  tint_utils_math
  tint_utils_memory
  tint_utils_reflection
  tint_utils_result
  tint_utils_rtti
  tint_utils_symbol
  tint_utils_text
  tint_utils_traits
)

endif(TINT_BUILD_GLSL_WRITER)
if(TINT_BUILD_GLSL_WRITER)
################################################################################
# Target:    tint_lang_glsl_writer_ast_raise_test
# Kind:      test
# Condition: TINT_BUILD_GLSL_WRITER
################################################################################
tint_add_target(tint_lang_glsl_writer_ast_raise_test test
  lang/glsl/writer/ast_raise/combine_samplers_test.cc
  lang/glsl/writer/ast_raise/pad_structs_test.cc
  lang/glsl/writer/ast_raise/texture_1d_to_2d_test.cc
  lang/glsl/writer/ast_raise/texture_builtins_from_uniform_test.cc
)

tint_target_add_dependencies(tint_lang_glsl_writer_ast_raise_test test
  tint_api_common
  tint_api_options
  tint_lang_core
  tint_lang_core_constant
  tint_lang_core_type
  tint_lang_wgsl
  tint_lang_wgsl_ast
  tint_lang_wgsl_ast_transform
  tint_lang_wgsl_ast_transform_test
  tint_lang_wgsl_program
  tint_lang_wgsl_reader
  tint_lang_wgsl_sem
  tint_lang_wgsl_writer
  tint_utils_containers
  tint_utils_diagnostic
  tint_utils_ice
  tint_utils_id
  tint_utils_macros
  tint_utils_math
  tint_utils_memory
  tint_utils_reflection
  tint_utils_result
  tint_utils_rtti
  tint_utils_symbol
  tint_utils_text
  tint_utils_traits
)

tint_target_add_external_dependencies(tint_lang_glsl_writer_ast_raise_test test
  "gtest"
)

if(TINT_BUILD_GLSL_WRITER)
  tint_target_add_dependencies(tint_lang_glsl_writer_ast_raise_test test
    tint_lang_glsl_writer_ast_raise
  )
endif(TINT_BUILD_GLSL_WRITER)

endif(TINT_BUILD_GLSL_WRITER)