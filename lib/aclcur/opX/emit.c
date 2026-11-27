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
#include "../util.h"
#include <ae2f/cc/assume.h>
#include <aclutil/wrdemit.h>

#if	__ACLSPV_CHECK_EXTRA
#define	s_assume(c, c_err) ae2f_ifezerr(c) { err_puts("assumed failed: " #c); return (c_err); }
#else
#define	s_assume(c, c_err)	ae2f_assume(c)
#endif

aclspv_compile_t EMIT_aclcur_opx(h_aclcur_t h, h_aclspv_ctx_t h_ctx) {

	aclspv_wrd_t	i = 0;
	aclspv_wrd_t	idx;
	aclspv_wrd_t	at;
	aclspv_wrd_t* ae2f_restrict wr;
	const aclspv_wrd_t	ANCHOUR = h_ctx->m_count.m_fnimpl;
	const aclspv_bool_t	IS_EXT = h->m_d.m_opx.m_op == SpvOpExtInst;
	const aclspv_wrd_t	NUM_OPRND = h->m_d.m_opx.m_num;

	/** a node with no type has no result, and the barriers are exactly that:
	 * [op|n][operands...] instead of [op|n][type][result][operands...].
	 * HEAD is the word count between the opcode word and the operands, the
	 * opcode word itself not included. */
	const aclspv_bool_t	IS_RESULT = h->m_ty != 0;
	const aclspv_wrd_t	HEAD = (IS_RESULT ? 2u : 0u) + (IS_EXT ? 2u : 0u);
	const aclspv_wrd_t	WORDCOUNT = 1u + HEAD + NUM_OPRND;

	s_assume(h, ACLSPV_COMPILE_UNHANDLED);
	s_assume(h->m_class == ACLCUR_OPX, ACLSPV_COMPILE_UNHANDLED);
	s_assume(h_ctx, ACLSPV_COMPILE_UNHANDLED);

	if(IS_RESULT) {
		s_assume(h->m_id, ACLSPV_COMPILE_UNHANDLED);
		s_assume(h->m_val == h->m_id, ACLSPV_COMPILE_UNHANDLED);
		s_assume(h->m_flag0 & aclcur_flag0(ACLCUR_FLAG0_VALUE), ACLSPV_COMPILE_UNHANDLED);

		unless(h->m_num_ref
				|| (h->m_flag0 & aclcur_flag0(ACLCUR_FLAG0_EFFECT)))
			return ACLSPV_COMPILE_OK;
	}
	else {
		/** a barrier has an effect by definition: it is never dropped. */
		s_assume(!h->m_val, ACLSPV_COMPILE_UNHANDLED);
		s_assume(!(h->m_flag0 & aclcur_flag0(ACLCUR_FLAG0_VALUE)), ACLSPV_COMPILE_UNHANDLED);
	}

	ae2f_ifezerr(h_ctx->m_count.m_fnimpl = aclutil_emit_wrd(
				&h_ctx->m_section.m_fnimpl
				, h_ctx->m_count.m_fnimpl + WORDCOUNT - 1
				, 0
				)) return ACLSPV_COMPILE_ALLOC_FAILED_CURSOR;

	/** the allocation above may have moved the buffer: resolve the words only
	 * now, and walk the operand chain (which lives in the cursor buffer, not
	 * here) afterwards. */
	wr = &aclutil_get_wrd_of_vec(&h_ctx->m_section.m_fnimpl)[ANCHOUR];
	wr[0] = aclspv_opcode_to_wrd(h->m_d.m_opx.m_op, WORDCOUNT - 1);

	at = 1;

	if(IS_RESULT) {
		wr[at++] = h->m_ty;
		wr[at++] = h->m_id;
	}

	if(IS_EXT) {
		wr[at++] = h->m_d.m_opx.m_set;
		wr[at++] = h->m_d.m_opx.m_inst;
	}

	idx = h->m_d.m_opx.m_o_head;
	while(idx) {
		const h_aclcur_t p_mbr = &s_get_buf(h_ctx)[idx];
#if __ACLSPV_CHECK_EXTRA
		ae2f_ifnezerr_strict(p_mbr->m_class != ACLCUR_OPX_MBR) return ACLSPV_COMPILE_UNHANDLED;
#endif
		wr[at + i] = p_mbr->m_d.m_opx_mbr.m_operand;
		idx = p_mbr->m_d.m_opx_mbr.m_nxt;
		++i;
	}

#if __ACLSPV_CHECK_EXTRA
	ae2f_ifnezerr_strict(i != h->m_d.m_opx.m_num) return ACLSPV_COMPILE_UNHANDLED;
#endif

	return ACLSPV_COMPILE_OK;
}

