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

#include <aclutil/scale.h>

#include "./op.h"
#include "./beforevisit.h"
#include "./beforevisit/util.h"
#include "./beforevisit/bool1.h"
#include "./beforevisit/cond.h"

aclspv_bool_t aclexpr_beforevisit(
		CXCursor					c_cur,
		struct aclexpr_visit_data* const ae2f_restrict	rdwr_data,
		aclspv_wrd_t					c_opidx
		) {
	h_scale_t LST_SCALE				= get_last_scale_from_vec(&CMDSTCK_SCALE(rdwr_data->m_ctx));
	aclspv_wrd_t* ae2f_restrict LST_SCALE_BUF	= get_buf_from_scale2(
			aclspv_wrd_t
			, &CMDSTCK_SCALE(CTX(rdwr_data))
			, *LST_SCALE
			);

	switch(LST_SCALE_BUF[ACLEXPR_OPBINCFG_OPCODE]) {
		case ACLEXPR_OP2_AND_L:
		case ACLEXPR_OP2_OR_L:
			return aclexpr_beforevisit2_bool1(
					c_cur
					, rdwr_data
					, c_opidx
					);

		case ACLEXPR_OP3_COND:
			return aclexpr_beforevisit3_cond(
					c_cur
					, rdwr_data
					, c_opidx
					);

		default:
			return 0;
	}

	ae2f_unreachable();
}
