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

#include "./op/none.h"
#include "./op.h"

#include "./class.h"
#include "./common.h"
#include "./util.h"

#include <aclval.h>

/** SpvOpControlBarrier / SpvOpMemoryBarrier: no Result Type, no Result.
 *
 * the node is an ACLCUR_OPX with a zero type and a zero value — i.e. not a
 * value — so EMIT_aclcur_opx skips the type / result words and the node needs
 * neither a new id nor a value.
 *
 * the words are scope / semantics values but the operands are id operands, so
 * each one is folded into an OpConstant first.
 *
 * reference is unconditional: an unreferenced barrier is still a barrier, and
 * unlike a store these operands are plain constants that carry no effect of
 * their own.
 * */
aclcur_id_t aclcur_cfg_barrier(
		enum SpvOp_		c_op,
		const aclspv_wrd_t	c_words[],
		aclspv_wrd_t		c_num,
		h_aclspv_ctx_t		h_ctx
		) {
	/** a barrier takes at most three scope words. */
	enum { S_MAX_WORD = 4 };

	aclspv_id_t		node;
	h_aclcur_t		p_node;
	aclspv_id_t		mbr_prv = 0;
	aclspv_wrd_t		i;
	aclspv_id_t		IDS[S_MAX_WORD];

	if(c_op != SpvOpControlBarrier && c_op != SpvOpMemoryBarrier) {
		err_puts("[aclcur::cfg::barrier] only a barrier has no result type");
		_aclctx_err(h_ctx, ACLSPV_COMPILE_MET_INVAL);
		return 0;
	}

	ae2f_ifnezerr(!c_num || !c_words) {
		err_puts("[aclcur::cfg::barrier] barrier needs its scope words");
		_aclctx_err(h_ctx, ACLSPV_COMPILE_PRM_INVAL_NIL);
		return 0;
	}

	if(c_num > S_MAX_WORD) {
		err_puts("[aclcur::cfg::barrier] too many scope words");
		_aclctx_err(h_ctx, ACLSPV_COMPILE_UNHANDLED_CARVE);
		return 0;
	}

	/** the operands are constants of the builtin's u32 parameter type. this
	 * runs before the node is allocated: a later instruction of the block must
	 * not end up in front of the barrier. */
	for(i = 0; i < c_num; ++i) {
		ae2f_ifezerr(IDS[i] = aclval_cfg_by_enum(c_words[i], h_ctx, ACLSPV_ID_U32))
			return 0;
	}

	ae2f_ifezerr(node = aclcur_cfg_node(0, h_ctx))
		return 0;

	ae2f_ifezerr(p_node = aclcur_get_node(node, h_ctx))
		return 0;

	p_node->m_class			= ACLCUR_OPX;
	p_node->m_val			= 0;
	p_node->m_ty			= 0;
	p_node->m_d.m_opx.m_op		= c_op;
	p_node->m_d.m_opx.m_num		= c_num;
	p_node->m_d.m_opx.m_o_head	= 0;
	p_node->m_d.m_opx.m_set		= 0;
	p_node->m_d.m_opx.m_inst	= 0;

	for(i = 0; i < c_num; ++i) {
		aclspv_id_t	mbr;
		h_aclcur_t	p_mbr;
		h_aclcur_t	p_prv;

		/** an operand-link node is never emitted: it is bookkeeping only. */
		ae2f_ifezerr(mbr = aclcur_cfg_node2(0, h_ctx, 1)) {
			/** undo the half-built node: it must not look like a barrier. */
			ae2f_ifezerr(p_node = aclcur_get_node(node, h_ctx)) return 0;
			p_node->m_d.m_opx.m_num = 0;
			return 0;
		}

		/** the cursor buffer may have moved: resolve both ends again. */
		ae2f_ifezerr(p_node = aclcur_get_node(node, h_ctx)) return 0;
		ae2f_ifezerr(p_mbr = aclcur_get_node(mbr, h_ctx)) return 0;

		p_mbr->m_class			= ACLCUR_OPX_MBR;
		p_mbr->m_d.m_opx_mbr.m_operand	= IDS[i];
		p_mbr->m_d.m_opx_mbr.m_nxt	= 0;

		if(!i) {
			p_node->m_d.m_opx.m_o_head
				= (aclspv_wrd_t)(p_mbr - s_get_buf(h_ctx));
		}
		else {
			ae2f_ifezerr(p_prv = aclcur_get_node(mbr_prv, h_ctx))
				return 0;
			p_prv->m_d.m_opx_mbr.m_nxt
				= (aclspv_wrd_t)(p_mbr - s_get_buf(h_ctx));
		}

		mbr_prv = mbr;
	}

	return node;
}
