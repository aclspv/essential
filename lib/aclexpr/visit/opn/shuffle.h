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

#ifndef	aclexpr_visit_opn_shuffle_h
#define	aclexpr_visit_opn_shuffle_h

#include <aclexpr.h>
#include <aclexpr/visit/api.h>
#include <aclspv.h>

/** handles `__builtin_shufflevector(v1, v2, i0, i1, ...)`.
 *
 * libclang surfaces the shuffle expression as a CXCursor_UnexposedExpr (the
 * dedicated AST node is not exposed), so detection is structural; see
 * aclexpr_shuffle_detect. only the two vector operands cascade; the mask
 * indices are literal constants that the carve recovers off the operator
 * cursor (the shuffle cursor is stashed on the scale's extra[2]).
 * */
aclexpr_visit_lbl_t aclexpr_visit_opn_shuffle(
		CXCursor				c_cur,
		struct aclexpr_visit_data* ae2f_restrict	rdwr_data,
		aclexpr_visit_lbl_t			c_lbl_success,
		aclexpr_visit_lbl_t			c_lbl_fail,
		aclexpr_visit_lbl_t			c_lbl_pass
		);

/** true when `c_cur` is a `__builtin_shufflevector` surfaced as an
 * UnexposedExpr: two operands followed by at least one integer-literal mask
 * component.
 *
 * the generic UnexposedExpr wrapper (an implicit cast) has exactly one
 * child, so the shape is unambiguous. the check is structural on purpose:
 * clang_getCursorType reports the spelling type (a typedef such as
 * `f32x4`, CXType_Typedef), not the canonical vector kind, so a kind test
 * would have to canonicalise first and still add nothing over the shape. */
aclspv_bool_t aclexpr_shuffle_detect(CXCursor c_cur);

#endif
