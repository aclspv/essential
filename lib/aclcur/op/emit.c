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

#include "./emit.h"
#include "../common.h"
#include <ae2f/cc/assume.h>
#include <aclutil/emitx.h>

#if	__ACLSPV_CHECK_EXTRA
#define	s_assume(c, c_err) ae2f_ifezerr(c) { err_puts("assumed failed: " #c); return (c_err); }
#else
#define	s_assume(c, c_err)	ae2f_assume(c)
#endif

aclspv_compile_t EMIT_aclcur_op1(h_aclcur_t h, h_aclspv_ctx_t h_ctx) {
	s_assume(h, ACLSPV_COMPILE_UNHANDLED);
	s_assume(h->m_class == ACLCUR_OP1, ACLSPV_COMPILE_UNHANDLED);
	s_assume(h->m_id, ACLSPV_COMPILE_UNHANDLED);
	s_assume(h->m_val == h->m_id, ACLSPV_COMPILE_UNHANDLED);
	s_assume(h->m_flag0 & aclcur_flag0(ACLCUR_FLAG0_VALUE), ACLSPV_COMPILE_UNHANDLED);
	s_assume(h_ctx, ACLSPV_COMPILE_UNHANDLED);

	unless(h->m_num_ref) return ACLSPV_COMPILE_OK;

	ae2f_ifezerr(h_ctx->m_count.m_fnimpl = aclutil_emitx_4(
				&h_ctx->m_section.m_fnimpl
				, h_ctx->m_count.m_fnimpl
				, (aclspv_wrd_t)h->m_d.m_op.m_op
				, h->m_ty
				, h->m_id
				, h->m_d.m_op.m_o[0]
				)) return ACLSPV_COMPILE_ALLOC_FAILED_CURSOR;

	return ACLSPV_COMPILE_OK;
}

aclspv_compile_t EMIT_aclcur_op2(h_aclcur_t h, h_aclspv_ctx_t h_ctx) {
	s_assume(h, ACLSPV_COMPILE_UNHANDLED);
	s_assume(h->m_class == ACLCUR_OP2, ACLSPV_COMPILE_UNHANDLED);
	s_assume(h->m_id, ACLSPV_COMPILE_UNHANDLED);
	s_assume(h->m_val == h->m_id, ACLSPV_COMPILE_UNHANDLED);
	s_assume(h->m_flag0 & aclcur_flag0(ACLCUR_FLAG0_VALUE), ACLSPV_COMPILE_UNHANDLED);
	s_assume(h_ctx, ACLSPV_COMPILE_UNHANDLED);

	unless(h->m_num_ref) return ACLSPV_COMPILE_OK;

	ae2f_ifezerr(h_ctx->m_count.m_fnimpl = aclutil_emitx_5(
				&h_ctx->m_section.m_fnimpl
				, h_ctx->m_count.m_fnimpl
				, (aclspv_wrd_t)h->m_d.m_op.m_op
				, h->m_ty
				, h->m_id
				, h->m_d.m_op.m_o[0]
				, h->m_d.m_op.m_o[1]
				)) return ACLSPV_COMPILE_ALLOC_FAILED_CURSOR;

	return ACLSPV_COMPILE_OK;
}

aclspv_compile_t EMIT_aclcur_op3(h_aclcur_t h, h_aclspv_ctx_t h_ctx) {
	s_assume(h, ACLSPV_COMPILE_UNHANDLED);
	s_assume(h->m_class == ACLCUR_OP3, ACLSPV_COMPILE_UNHANDLED);
	s_assume(h->m_id, ACLSPV_COMPILE_UNHANDLED);
	s_assume(h->m_val == h->m_id, ACLSPV_COMPILE_UNHANDLED);
	s_assume(h->m_flag0 & aclcur_flag0(ACLCUR_FLAG0_VALUE), ACLSPV_COMPILE_UNHANDLED);
	s_assume(h_ctx, ACLSPV_COMPILE_UNHANDLED);

	unless(h->m_num_ref) return ACLSPV_COMPILE_OK;

	ae2f_ifezerr(h_ctx->m_count.m_fnimpl = aclutil_emitx_6(
				&h_ctx->m_section.m_fnimpl
				, h_ctx->m_count.m_fnimpl
				, (aclspv_wrd_t)h->m_d.m_op.m_op
				, h->m_ty
				, h->m_id
				, h->m_d.m_op.m_o[0]
				, h->m_d.m_op.m_o[1]
				, h->m_d.m_op.m_o[2]
				)) return ACLSPV_COMPILE_ALLOC_FAILED_CURSOR;

	return ACLSPV_COMPILE_OK;
}

/** [op|3][o0][o1] — no Result Type, no Result (SpvOpStore).
 *
 * deliberately no m_num_ref gate, unlike the opN emitters above: a store
 * has side effects and must never be dropped as unreferenced.
 * */
aclspv_compile_t EMIT_aclcur_st(h_aclcur_t h, h_aclspv_ctx_t h_ctx) {
	s_assume(h, ACLSPV_COMPILE_UNHANDLED);
	s_assume(h->m_class == ACLCUR_ST, ACLSPV_COMPILE_UNHANDLED);
	s_assume(h->m_id, ACLSPV_COMPILE_UNHANDLED);
	s_assume(h_ctx, ACLSPV_COMPILE_UNHANDLED);

	ae2f_ifezerr(h_ctx->m_count.m_fnimpl = aclutil_emitx_3(
				&h_ctx->m_section.m_fnimpl
				, h_ctx->m_count.m_fnimpl
				, (aclspv_wrd_t)h->m_d.m_op.m_op
				, h->m_d.m_op.m_o[0]
				, h->m_d.m_op.m_o[1]
				)) return ACLSPV_COMPILE_ALLOC_FAILED_CURSOR;

	return ACLSPV_COMPILE_OK;
}
