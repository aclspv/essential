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

#ifndef	aclcur_common_h
#define aclcur_common_h

#include <acldef.h>
#include <aclty/common.h>

#include "../aclcur.h"
#include "./lbl.h"
#include "./j.h"
#include "./jc.h"
#include "./sw.h"
#include "./reg.h"
#include "./class.h"
#include "./load.h"
#include "./phi.h"
#include "./store.h"
#include "./op.h"
#include "./opX.h"
#include "./emit.h"

/**
 * one node of the per-function cursor buffer.
 *
 * APPEND-ONLY. a node is only ever appended (aclcur_cfg_node2), at the end of
 * the buffer, and nothing is removed, moved or reordered afterwards. the two
 * things that look like shrinking are not:
 *
 *  - m_num_cursor is reset to 0 when a function's emission is bootstrapped
 *    (aclemit/root.c, aclemit/bodies.c) and aclcur_cfg_root then appends the
 *    root label as node 0. the buffer therefore describes ONE function, and an
 *    index is meaningless outside it.
 *  - m_num_skip / m_num_skip_prv are word counts over the EMITTED SPIR-V
 *    stream: they say how many words the next/previous node's instruction
 *    occupies, so a node whose instruction was dropped can be walked past. no
 *    node is deleted and no slot is reused.
 *
 * what that buys: the physical index is an immutable identity for the lifetime
 * of the function's cursor scope, and aclcur_get_node_idx() resolves one in
 * O(1) — where aclcur_get_node() has to scan for the id.
 *
 * two caveats, both load-bearing:
 *
 *  - a POINTER into the buffer (h_aclcur_t) is invalidated when the buffer
 *    grows, because growth reallocates it. only indices stay put; re-fetch by
 *    id/index after anything that may append (every aclcur_cfg_* call).
 *  - a node appended for a RESERVED id is claimed later: aclcur_cfg_node2 hands
 *    back the id of a node still classified ACLCUR_RESERVED and the caller
 *    then configures it. so an index means one node only from the moment its
 *    class is set — do not stamp an identity onto a slot that is still
 *    reserved.
 *
 * node 0 is always the root label, so no register is ever at index 0.
 * */
struct aclcur {
	union {
		aclcur_lbl_t	m_lbl;
		aclcur_j_t	m_j;
		aclcur_jc_t	m_jc;
		aclcur_sw_t	m_sw;
		aclcur_sw_case_t m_sw_case;
		aclcur_reg_t	m_reg;
		aclcur_load_t	m_load;
		aclcur_store_t	m_store;
		aclcur_phi_t	m_phi;
		aclcur_op_t	m_op;
		aclcur_opx_t	m_opx;
		aclcur_opx_mbr_t	m_opx_mbr;
	}		m_d;

	CXCursor	m_cur;

	/** enum ACLCUR_FLAG0_ */
	aclspv_wrd_t	m_flag0;

	/** the node's id: a compile-time handle, NOT the node's index in the
	 * buffer (the layout is append-only, so the index could serve as one —
	 * see struct aclcur). it doubles as the register's SPIR-V result id for a
	 * register that owns storage (aclcur_regvar_emit_reg emits
	 * `OpVariable <ty> <m_id>`), which is why the entry-point argument path
	 * and the function-parameter path patch it to the id they emitted. */
	aclcur_id_t	m_id;

	/** current label */
	aclspv_id_t	m_lbl;

	/** enum ACLCUR_ */
	aclspv_wrd_t	m_class;
	aclspv_id_t	m_val;

	/** 
	 * type of m_val.
	 *
	 * special: for 0th element(root label), this is a return type of the function.
	 * */
	aclty_id_t	m_ty;

	/** number of cursor referenced. */
	aclspv_wrd_t	m_num_ref;

	/** number of word count to be skipped to get next cursor. */
	aclspv_wrd_t	m_num_skip;
	/** number of word count to be skipped to get previous cursor */
	aclspv_wrd_t	m_num_skip_prv;

	/** depth of this cursor */
	aclspv_wrd_t	m_depth;

	/** a value tracked on current branch (label) */
	acltrl_t m_val_lit;

	/** 
	 * literal mask for predicted value.
	 * see enum ACLUTIL_LITERAL_
	 * */
	aclspv_wrd_t	m_val_mask;

	/** this VAR's write generation (meaningful on ACLCUR_FLAG0_REG_VAR): the
	 * count of stores that landed in it. a cached value of any register naming
	 * this var is current only while the generation it recorded still
	 * matches. */
	aclspv_wrd_t	m_gen;

	/** the var generation m_val was filled at (0 when the register names no
	 * var: nothing to compare, the alias watermark decides). */
	aclspv_wrd_t	m_val_gen;

	/** the m_gen_alias m_val was filled at, and the point an untrack resyncs
	 * to — so "an alias write happened since we last looked" is expressible
	 * without walking anything. */
	aclspv_wrd_t	m_val_alias;
};

aclcur_id_t aclcur_cfg_node2(aclspv_id_reserved_t c_newid, h_aclspv_ctx_t h_ctx, aclspv_bool_t c_toskip);

/** 
 * this assigns new id
 * assumes skipping is not happening
 *
 * see aclcur_cfg_node2
 * */
static ae2f_inline_f aclcur_id_t aclcur_cfg_node(aclspv_id_reserved_t c, h_aclspv_ctx_t h_ctx) {
	return aclcur_cfg_node2(c, h_ctx, 0);
}

ae2f_ccpure aclcur_t*	aclcur_get_node(aclcur_id_t c, const x_aclspv_ctx* ae2f_restrict h_ctx);
ae2f_ccpure aclcur_t* aclcur_get_node_prv(aclcur_id_t c, const x_aclspv_ctx* ae2f_restrict h_ctx);
ae2f_ccpure aclcur_t* aclcur_get_node_nxt(aclcur_id_t c, const x_aclspv_ctx* ae2f_restrict h_ctx);

/** O(1) node access through its cursor-buffer index.
 * returns 0 when the index is out of range or ACLCUR_IDX_INVAL. */
ae2f_ccpure aclcur_t* aclcur_get_node_idx(
		aclcur_idx_t			c_idx,
		const x_aclspv_ctx* ae2f_restrict h_ctx
		);

/** O(n) reverse lookup: index of the node carrying `c_id`.
 * returns ACLCUR_IDX_INVAL when no node has it. */
ae2f_ccpure aclcur_idx_t aclcur_idx_of(
		aclcur_id_t			c_id,
		const x_aclspv_ctx* ae2f_restrict h_ctx
		);

/** id of the node at `c_idx`, or 0 when the index is invalid. */
ae2f_ccpure aclcur_id_t aclcur_id_of_idx(
		aclcur_idx_t			c_idx,
		const x_aclspv_ctx* ae2f_restrict h_ctx
		);

/** 
 * sets the c's cursor to c_cur.
 * when c_cur is null cursor it changes none of c.
 *
 * returns
 * : null cursor `clang_getNullCursor` when no id was found.
 * : a cursor it have previously had.
 * */
CXCursor aclcur_set_cxcursor(aclcur_id_t c, const CXCursor c_cur, h_aclspv_ctx_t h_ctx);

#endif
