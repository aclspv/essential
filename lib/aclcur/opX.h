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

#ifndef aclcur_opX_h
#define aclcur_opX_h

#include <acldef.h>
#include <aclty.h>
#include <aclctx/id.h>
#include <aclcur.h>

/** @brief operand-link node for >3-operand (opX) ops. chained via cursor-buffer index, like aclty mbr nodes. */
typedef struct {
	aclspv_id_t	m_operand;
	/** @brief next link node's cursor-buffer index. 0 = terminator. */
	aclspv_wrd_t	m_nxt;
} aclcur_opx_mbr_t;


/** @brief op with variable operand count. operands form a linked list of aclcur_opx_mbr_t. */
typedef struct {
	enum SpvOp_	m_op;
	/** @brief cursor-buffer index of the first operand-link node. 0 = none. */
	aclspv_wrd_t	m_o_head;
	/** @brief operand count. */
	aclspv_wrd_t	m_num;

	/** @brief for SpvOpExtInst: the imported instruction-set id. */
	aclspv_id_t	m_set;
	/** @brief for SpvOpExtInst: the instruction number inside the set. */
	aclspv_wrd_t	m_inst;
} aclcur_opx_t;

aclcur_vid_t aclcur_cfg_opx(
		aclspv_id_reserved_t	c_newid_opt,
		enum SpvOp_	c_op,
		aclty_id_t	c_ty,
		const aclspv_vid_t	c_os[],
		aclspv_wrd_t	c_num,
		h_aclspv_ctx_t	h_ctx
		);

/** SpvOpFunctionCall: [type][result][function][args...].
 *
 * c_fn is the callee's OpFunction result id; c_args are value ids already
 * converted to the callee's parameter types. never deduplicated: a call has
 * effects, so two identical calls in different blocks must stay two
 * instructions. */
aclspv_id_t aclcur_cfg_call(
		aclspv_id_t		c_fn,
		aclty_id_t		c_ty,
		const aclspv_vid_t	c_args[],
		aclspv_wrd_t		c_num,
		h_aclspv_ctx_t		h_ctx
		);

/** SpvOpExtInst: [type][result][set id][instruction][operands...] */
aclspv_id_t aclcur_cfg_extinst(
		aclspv_id_reserved_t	c_newid_opt,
		aclty_id_t		c_ty,
		aclspv_id_t		c_set,
		aclspv_wrd_t		c_inst,
		const aclspv_vid_t	c_os[],
		aclspv_wrd_t		c_num,
		h_aclspv_ctx_t		h_ctx
		);

/** SpvOpVectorShuffle: [type][result][v1][v2][component words...].
 *
 * the components are literal words (indices into the concatenation of v1
 * and v2), not id operands: they are emitted verbatim. */
aclspv_id_t aclcur_cfg_shuffle(
		aclspv_id_reserved_t	c_newid_opt,
		aclty_id_t		c_ty,
		aclspv_id_t		c_v1,
		aclspv_id_t		c_v2,
		const aclspv_wrd_t*	c_comps,
		aclspv_wrd_t		c_num,
		h_aclspv_ctx_t		h_ctx
		);

#include "./opX/emit.h"

#endif

