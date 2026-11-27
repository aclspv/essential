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

#include <aclspv.h>

#include "../../lib/aclcur/common.h"
#include "../../lib/aclcur/reg/var.h"
#include "../../lib/aclcur/lbl/pure.h"

#include <stdio.h>
#include <string.h>

#define	CHECK(c)	do { \
	if(!(c)) { \
		fprintf(stderr, "check failed: %s:%d\n", __FILE__, __LINE__); \
		return 1; \
	} \
} while(0)

/** carve a bare register-bound access in the currently open label. the
 * load / store carving also fixes m_lbl; the analysis only reads m_class
 * and the register operand. */
static aclspv_id_t s_mk_access(
		aclspv_wrd_t		c_class,
		aclspv_id_t		c_reg,
		h_aclspv_ctx_t		h_ctx
		) {
	aclspv_id_t	id;
	h_aclcur_t	p;

	ae2f_ifezerr(id = aclcur_cfg_node(0, h_ctx))
		return 0;

	ae2f_ifezerr(p = aclcur_get_node(id, h_ctx))
		return 0;

	p->m_class = c_class;

	if(c_class == ACLCUR_LOAD)
		p->m_d.m_load.m_reg = c_reg;
	else
		p->m_d.m_store.m_reg = c_reg;

	return id;
}

/** open a label the way the frontend does: reserve the id, carve the jump
 * that targets it in the current label, then carve the label itself. the
 * jump is what makes the block emit-live (see aclcur_ssa_lbl_live). */
static aclcur_lblid_t s_open_lbl(h_aclspv_ctx_t h_ctx) {
	aclspv_id_t c_lbl = h_ctx->m_id++;

	ae2f_ifezerr(aclcur_cfg_jmp(c_lbl, h_ctx))
		return 0;

	return (aclcur_lblid_t)aclcur_cfg_lbl(c_lbl, h_ctx);
}

int main(void) {
	x_aclspv_ctx	ctx;
	aclspv_usrcfg_t	cfg;

	aclcur_lblid_t	lbl_a;
	aclcur_lblid_t	lbl_b;
	aclcur_lblid_t	lbl_c;
	aclspv_id_t	reg_a;
	aclspv_id_t	reg_c;
	aclspv_id_t	reg_ext;
	aclspv_id_t	reg_view;
	const aclcur_reg_t	regin = aclcur_regvar_in();

	memset(&cfg, 0, sizeof(cfg));

	CHECK(!aclspv_init_ctx(&ctx, &cfg));

	CHECK(aclcur_cfg_root(ACLSPV_ID_VOID, &ctx));

	/** block A: defines reg_a and reads it. self-contained. */
	CHECK(lbl_a = s_open_lbl(&ctx));
	CHECK(reg_a = aclcur_cfg_reg(
				clang_getNullCursor()
				, ACLSPV_ID_I32
				, SpvStorageClassFunction
				, &regin
				, &ctx));
	CHECK(s_mk_access(ACLCUR_LOAD, reg_a, &ctx));
	CHECK(aclcur_lbl_cfg_pure(lbl_a, &ctx) == ACLSPV_COMPILE_OK);
	CHECK(aclcur_lbl_is_const(lbl_a, &ctx));
	CHECK(aclcur_lbl_is_pure(lbl_a, &ctx));

	/** block B: reg_a belongs to A. reading it makes B noconstant, it is
	 * not defined by B; writing it makes B nopure. */
	CHECK(lbl_b = s_open_lbl(&ctx));
	CHECK(s_mk_access(ACLCUR_LOAD, reg_a, &ctx));
	CHECK(aclcur_lbl_cfg_pure(lbl_b, &ctx) == ACLSPV_COMPILE_OK);
	CHECK(!aclcur_lbl_is_const(lbl_b, &ctx));
	CHECK(aclcur_lbl_is_pure(lbl_b, &ctx));

	CHECK(s_mk_access(ACLCUR_STORE, reg_a, &ctx));
	CHECK(!aclcur_lbl_is_pure(lbl_b, &ctx));
	/** the predicate recomputes: the earlier const answer must not have
	 * been latched into a stale snapshot. */
	CHECK(!aclcur_lbl_is_const(lbl_b, &ctx));

	/** block C: defines and writes its own register only. */
	CHECK(lbl_c = s_open_lbl(&ctx));
	CHECK(reg_c = aclcur_cfg_reg(
				clang_getNullCursor()
				, ACLSPV_ID_I32
				, SpvStorageClassFunction
				, &regin
				, &ctx));
	CHECK(s_mk_access(ACLCUR_STORE, reg_c, &ctx));
	CHECK(aclcur_lbl_cfg_pure(lbl_c, &ctx) == ACLSPV_COMPILE_OK);
	CHECK(aclcur_lbl_is_const(lbl_c, &ctx));
	CHECK(aclcur_lbl_is_pure(lbl_c, &ctx));

	/** function level: every access so far targets Function-storage
	 * registers, which the function owns, so the label-level noconstant /
	 * nopure of B do not leak into the function answer. */
	CHECK(aclcur_fn_cfg_pure(&ctx) == ACLSPV_COMPILE_OK);
	CHECK(aclcur_fn_is_const(&ctx));
	CHECK(aclcur_fn_is_pure(&ctx));

	/** an interface buffer is not the function's storage. a bare LOAD of
	 * the resource ROOT is address materialization (reading `in` yields
	 * the pointer) and does not make the function non-const. */
	CHECK(reg_ext = aclcur_cfg_reg(
				clang_getNullCursor()
				, ACLSPV_ID_I32
				, SpvStorageClassStorageBuffer
				, &regin
				, &ctx));
	CHECK(s_mk_access(ACLCUR_LOAD, reg_ext, &ctx));
	CHECK(aclcur_fn_is_const(&ctx));

	/** a read through a view of the buffer is a read of state. the view
	 * node is carved in C too, so the label rule still calls it local —
	 * the blind spot the function rule closes by keying on the storage
	 * class instead. */
	CHECK(reg_view = aclcur_cfg_reg(
				clang_getNullCursor()
				, ACLSPV_ID_I32
				, SpvStorageClassStorageBuffer
				, &regin
				, &ctx));
	CHECK(aclcur_reg_alias2(reg_view, reg_ext, &ctx) == reg_view);
	CHECK(s_mk_access(ACLCUR_LOAD, reg_view, &ctx));
	CHECK(aclcur_lbl_is_const(lbl_c, &ctx));
	CHECK(!aclcur_fn_is_const(&ctx));
	CHECK(aclcur_fn_is_pure(&ctx));

	CHECK(s_mk_access(ACLCUR_STORE, reg_ext, &ctx));
	CHECK(!aclcur_fn_is_pure(&ctx));

	aclspv_fini_ctx(&ctx);
	return 0;
}
