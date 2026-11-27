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

#ifndef	aclexpr_carve_util_h
#define	aclexpr_carve_util_h

#include <aclexpr.h>
#include <aclexpr/op.h>
#include <aclcur/common.h>

/** cast the RHS value to an explicit type id.
 * returns 0 on success, nonzero when the conversion is not possible. */
aclspv_bool_t aclexpr_carve_cast_to_ty(
		aclspv_id_t			c_ty,
		aclspv_id_t* ae2f_restrict	rw_val,
		aclexpr_opbincfg_extra* ae2f_restrict rw_extra,
		h_aclspv_ctx_t			h_ctx
		);

/** cast the RHS value to the storage type of `c_reg`.
 * returns 0 on success, nonzero when the conversion is not possible. */
aclspv_bool_t aclexpr_carve_cast_to_reg(
		aclcur_regid_t			c_reg,
		aclspv_id_t* ae2f_restrict	rw_val,
		aclexpr_opbincfg_extra* ae2f_restrict rw_extra,
		h_aclspv_ctx_t			h_ctx
		);

#define CTX(rdwr_data)			(rdwr_data)->m_ctx
#define EXPRSTCK_COUNT(rdwr_data)	(rdwr_data)->m_exprcount
#define	RET_LITERAL_REF(rdwr_data)	(rdwr_data)->m_ret_literal

#define	CMDSTCK_SCALE(CTX)	(CTX)->m_body.m_bootscale
#define	CMDCOUNTERSTCK(CTX)	(CTX)->m_body.m_cmdcountstck

#endif
