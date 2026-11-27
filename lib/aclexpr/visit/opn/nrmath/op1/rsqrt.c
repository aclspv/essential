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

#include "./rsqrt.h"

#if	__ACLSPV_MATH_NR

#include <aclutil/scale.h>
#include <util/wrdemit.h>
#include <aclexpr/op.h>

enum S_RET_ aclexpr_visit_op1_rsqrtf_nr(
		const CXCursor c_cur,
		ae2f_unused const h_aclspv_ctx_t			h_ctx,
		x_aclspv_vec* ae2f_restrict const h_cmdscale
		) {
	x_scale* ae2f_restrict	NEW_SCALE;
	aclspv_wrd_t* ae2f_restrict VEC;

	ae2f_ifnezerr_strict(clang_Cursor_getNumArguments(c_cur) != 1)
		return S_RET_FAIL;

	NEW_SCALE = get_last_scale_from_vec(h_cmdscale);
	NEW_SCALE = mk_scale_from_vec(
			h_cmdscale
			, 
			aclutil_count_to_sz(ACLEXPR_OPBINCFG_END + 1)
			+ (sizeof(aclexpr_opbincfg_extra))
			);

	ae2f_ifezerr(NEW_SCALE) {
		return S_RET_FAIL;
	}

	VEC = get_buf_from_scale2(aclspv_wrd_t, h_cmdscale, *NEW_SCALE);

	ae2f_ifezerr(VEC)
		return S_RET_FAIL;

	VEC[ACLEXPR_OPBINCFG_NUMOPRND]	= 1;				/** number of operands */
	VEC[ACLEXPR_OPBINCFG_OPCODE]	= ACLEXPR_OP1_RSQRTF_NR;
	VEC[ACLEXPR_OPBINCFG_RESTYID]	= 0;

	aclmemzero(&VEC[ACLEXPR_OPBINCFG_END + 1], sizeof(aclexpr_opbincfg_extra));

	return S_RET_GOOD;
}

#endif
