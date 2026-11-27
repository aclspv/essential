/*
 * Copyright 2026 dalmurii
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * See the LICENSE file for the full text, including LLVM Exceptions.
 */

#ifndef	aclexpr_visit_op1_glslmath_h
#define	aclexpr_visit_op1_glslmath_h

#include <aclexpr.h>
#include <aclexpr/visit/api.h>
#include <aclspv.h>

/** handles the GLSL.std.450 unary float math builtins, all of which take
 * exactly one argument:
 *
 *   __builtin_sinf / __builtin_cosf / __builtin_tanf      (f32)
 *   __builtin_sinf16 / __builtin_cosf16 / __builtin_tanf16 (f16)
 *   __builtin_elementwise_sin / __builtin_elementwise_cos /
 *   __builtin_elementwise_tan                             (f32 scalar/vector)
 *   __builtin_floorf / __builtin_floorf16 / __builtin_elementwise_floor
 *
 * the spellings share one visitor: the builtin's own signature fixes the
 * expected operand type (f32 for `*f`, f16 for `*f16`), and the generic
 * elementwise spelling expects a simd float. the expected type rides the
 * scale's RESTYID slot; the carve does the conversion.
 * */
aclexpr_visit_lbl_t aclexpr_visit_op1_glsl_math(
		CXCursor				c_cur,
		struct aclexpr_visit_data* ae2f_restrict	rdwr_data,
		aclexpr_visit_lbl_t			c_lbl_success,
		aclexpr_visit_lbl_t			c_lbl_fail,
		aclexpr_visit_lbl_t			c_lbl_pass
		);

#endif
