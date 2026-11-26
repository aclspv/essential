#include "./inline.h"

#include <aclctx/err.h>
#include <aclemit/body.h>
#include <aclemit/dtor.h>
#include <aclsym/fn.h>

#include <aclcur/common.h>
#include <aclcur/reg.h>
#include <aclcur/reg/var.h>
#include <aclcur/reg/view.h>
#include <aclcur/jc.h>
#include <aclcur/ret.h>
#include <aclcur/ssa.h>

#include <aclty.h>
#include <aclty/fn/arg.h>
#include <aclval.h>
#include <aclval/nil.h>
#include <aclutil/fn.h>
#include <aclutil/scale.h>
#include <aclmem.h>

struct s_body_pick { CXCursor m_body; };

static enum CXChildVisitResult s_body_pick_visit(
		CXCursor c, ae2f_unused CXCursor p, CXClientData rw) {
	struct s_body_pick* ae2f_restrict b = rw;
	if(clang_getCursorKind(c) == CXCursor_CompoundStmt) {
		b->m_body = c;
		return CXChildVisit_Break;
	}
	return CXChildVisit_Continue;
}

static CXCursor s_body_of(CXCursor c_fn) {
	struct s_body_pick B;
	B.m_body = clang_getNullCursor();
	clang_visitChildren(c_fn, s_body_pick_visit, &B);
	return B.m_body;
}

struct s_shape {
	aclspv_wrd_t	m_children;
	aclspv_wrd_t	m_returns;
	aclspv_bool_t	m_last_ret;
};

static enum CXChildVisitResult s_shape_direct(
		CXCursor c, ae2f_unused CXCursor p, CXClientData rw) {
	struct s_shape* ae2f_restrict S = rw;
	++S->m_children;
	S->m_last_ret = (aclspv_bool_t)(clang_getCursorKind(c) == CXCursor_ReturnStmt);
	return CXChildVisit_Continue;
}

static enum CXChildVisitResult s_shape_all(
		CXCursor c, ae2f_unused CXCursor p, CXClientData rw) {
	struct s_shape* ae2f_restrict S = rw;
	if(clang_getCursorKind(c) == CXCursor_ReturnStmt)
		++S->m_returns;
	return CXChildVisit_Recurse;
}

/* the declaration whose body is being carved right now, whatever started
 * it: the body of a call site's callee is its own callee.
 *
 * it lives here rather than in the context because a call site inside an
 * INLINED body names the CALLEE's parameter cursors, not the caller's: an
 * argument that binds a pointer parameter has to reach the callee's own
 * cursor, or the callee's `p->m` resolves against a cursor it never saw. */
static CXCursor s_body_fn;

CXCursor aclemit_inline_callee(h_aclspv_ctx_t h_ctx) {
	(void)h_ctx;
	return s_body_fn;
}

static aclspv_bool_t s_has_return(CXCursor c_body) {
	struct s_shape S;
	if(clang_Cursor_isNull(c_body)) return 0;
	S.m_children = 0; S.m_returns = 0; S.m_last_ret = 0;
	clang_visitChildren(c_body, s_shape_all, &S);
	return (aclspv_bool_t)(S.m_returns != 0);
}

struct s_ret_loop {
	aclspv_bool_t	m_in_loop;
	aclspv_bool_t	m_found;
};

/** does any ReturnStmt sit under a loop? the inline return path lowers a
 * return to a branch to the outer inline loop's continue target, which is
 * only a structured exit for selection constructs, not for nested loops. */
static enum CXChildVisitResult s_ret_loop_visit(
		CXCursor c, ae2f_unused CXCursor p, CXClientData rw) {
	struct s_ret_loop* ae2f_restrict S = rw;
	enum CXChildVisitResult RES;
	struct s_ret_loop CHILD;
	const enum CXCursorKind KIND = clang_getCursorKind(c);

	if(S->m_found)
		return CXChildVisit_Break;

	if(KIND == CXCursor_ReturnStmt) {
		if(S->m_in_loop)
			S->m_found = 1;
		return S->m_found ? CXChildVisit_Break : CXChildVisit_Continue;
	}

	CHILD = *S;
	if(KIND == CXCursor_ForStmt
			|| KIND == CXCursor_WhileStmt
			|| KIND == CXCursor_DoStmt)
		CHILD.m_in_loop = 1;

	RES = clang_visitChildren(c, s_ret_loop_visit, &CHILD);
	if(CHILD.m_found)
		S->m_found = 1;

	(void)RES;
	return S->m_found ? CXChildVisit_Break : CXChildVisit_Continue;
}

static aclspv_bool_t s_has_return_in_loop(CXCursor c_body) {
	struct s_ret_loop S;
	if(clang_Cursor_isNull(c_body)) return 0;
	S.m_in_loop = 0;
	S.m_found = 0;
	clang_visitChildren(c_body, s_ret_loop_visit, &S);
	return S.m_found;
}


aclspv_bool_t aclemit_inline_supported(CXCursor c_fn) {
	CXCursor B = s_body_of(c_fn);
	struct s_shape S;
	if(clang_Cursor_isNull(B)) return 0;
	S.m_children = 0; S.m_returns = 0; S.m_last_ret = 0;
	clang_visitChildren(B, s_shape_direct, &S);
	clang_visitChildren(B, s_shape_all, &S);

	/** returns under a loop still fall back to OpFunctionCall: the
	 * single-iteration loop exit is not a structured exit from a nested
	 * loop. */
	if(s_has_return_in_loop(B))
		return 0;

	return 1;
}


static aclspv_bool_t s_park_expr(h_aclspv_ctx_t h_ctx) {
	if(aclmem_vecstck_push(&h_ctx->m_body.m_bootscale,
				&h_ctx->m_body.m_bootscale_stck,
				&h_ctx->m_body.m_num_bootscale_stck)) return 0;
	if(aclmem_vecstck_push(&h_ctx->m_body.m_cmdcountstck,
				&h_ctx->m_body.m_cmdcountstck_stck,
				&h_ctx->m_body.m_num_cmdcountstck_stck)) {
		aclmem_vecstck_pop(&h_ctx->m_body.m_bootscale,
				&h_ctx->m_body.m_bootscale_stck,
				&h_ctx->m_body.m_num_bootscale_stck);
		return 0;
	}
	return 1;
}

static void s_unpark_expr(h_aclspv_ctx_t h_ctx) {
	aclmem_vecstck_pop(&h_ctx->m_body.m_cmdcountstck,
			&h_ctx->m_body.m_cmdcountstck_stck,
			&h_ctx->m_body.m_num_cmdcountstck_stck);
	aclmem_vecstck_pop(&h_ctx->m_body.m_bootscale,
			&h_ctx->m_body.m_bootscale_stck,
			&h_ctx->m_body.m_num_bootscale_stck);
}

aclspv_bool_t aclemit_inline_merge_push(h_aclspv_ctx_t h_ctx, aclspv_id_t c_merge) {
	if(!h_ctx || h_ctx->m_inline.m_merge_depth >= 16) return 0;
	unless(h_ctx->m_inline.m_active) return 1;
	h_ctx->m_inline.m_merge[h_ctx->m_inline.m_merge_depth++] = c_merge;
	return 1;
}

void aclemit_inline_merge_pop(h_aclspv_ctx_t h_ctx) {
	if(h_ctx && h_ctx->m_inline.m_active && h_ctx->m_inline.m_merge_depth)
		--h_ctx->m_inline.m_merge_depth;
}

aclspv_id_t aclemit_inline_return_target(h_aclspv_ctx_t h_ctx) {
	if(!h_ctx) return 0;
	return h_ctx->m_inline.m_lbl_end;
}

aclspv_bool_t aclemit_inline_gate(h_aclspv_ctx_t h_ctx) {
	aclspv_id_t c_cond, c_cont, c_target;
	if(!h_ctx || !h_ctx->m_inline.m_active || !h_ctx->m_inline.m_reg_returned)
		return 1;
	ae2f_ifezerr(c_cond = aclcur_load_reg(h_ctx->m_inline.m_reg_returned, h_ctx, 0))
		return 0;
	c_target = aclemit_inline_return_target(h_ctx);
	c_cont = h_ctx->m_id++;
	if(!aclcur_cfg_jc(c_cond, c_target, c_cont, 0, 0, c_cont, 0, 0, h_ctx))
		return 0;
	if(!aclcur_cfg_lbl(c_cont, h_ctx))
		return 0;
	h_ctx->m_inline.m_terminated = 0;
	return 1;
}

aclspv_bool_t aclemit_inline_fn(
		CXCursor c_fn, CXCursor c_body,
		const aclspv_id_t* c_args, aclspv_wrd_t c_num,
		aclspv_id_t* wr_ret, h_aclspv_ctx_t h_ctx) {
	aclty_t* ae2f_restrict p_fn;
	aclspv_id_t id_fn_ty, id_ret = 0, id_end = 0, id_flag = 0, id_header = 0, id_body = 0, id_cont = 0;
	aclty_id_t ty_ret;
	aclspv_wrd_t i, c_dtor_base;
	aclspv_bool_t c_has_this, c_has_ret, c_fnret;
	aclspv_id_t id_brk, id_cnt, id_this;
	aclspv_bool_t old_active, old_branched, old_terminated, old_fnterm;
	aclspv_id_t old_end, old_ret, old_ty, old_retflag;
	CXCursor old_callee;
	aclspv_wrd_t old_base, old_depth, old_merge;
	aclemit_body_t DAT;
	h_scale_t S;

	ae2f_ifezerr(h_ctx) return 0;
	if(h_ctx->m_inline.m_depth >= 16) return 0;
	if(clang_Cursor_isNull(c_body)) c_body = s_body_of(c_fn);
	if(clang_Cursor_isNull(c_body)) return 0;
	ae2f_ifezerr(id_fn_ty = aclty_mk_fn1_this(c_fn, clang_getCursorType(c_fn), h_ctx)) return 0;
	ae2f_ifezerr(p_fn = aclty_find(id_fn_ty, h_ctx)) return 0;
	if(p_fn->m_info.m_fn.m_num_arg != c_num) return 0;

	ty_ret = p_fn->m_info.m_fn.m_ret_id;
	c_has_this = aclutil_fn_has_this(c_fn);
	c_has_ret = s_has_return(c_body);
	c_fnret = aclcur_get_fnret(h_ctx);
	id_brk = aclcur_get_break_loop(h_ctx);
	id_cnt = aclcur_get_continue_loop(h_ctx);
	id_this = h_ctx->m_this;
	S = get_last_scale_from_vec(&h_ctx->m_dtor_scope);
	c_dtor_base = S ? S->m_id : 0;
	if(!aclemit_dtor_push(h_ctx)) return 0;

	if(c_has_ret) {
		const aclcur_reg_t REGIN = aclcur_regvar_in();
		id_end = h_ctx->m_id++;
		id_header = h_ctx->m_id++;
		id_body = h_ctx->m_id++;
		id_cont = h_ctx->m_id++;
		ae2f_ifezerr(id_flag = aclcur_cfg_reg(clang_getNullCursor(),
					ACLSPV_ID_BOOL, SpvStorageClassFunction, &REGIN, h_ctx)) return 0;
		if(!aclcur_store_reg(id_flag, aclval_cfg_bool(1, h_ctx), h_ctx)) return 0;
	}

	if(ty_ret != ACLSPV_ID_VOID) {
		const aclcur_reg_t REGIN = aclcur_regvar_in();
		ae2f_ifezerr(id_ret = aclcur_cfg_reg(clang_getNullCursor(),
					ty_ret, SpvStorageClassFunction, &REGIN, h_ctx)) return 0;
		{
			h_aclcur_t p = aclcur_get_node(id_ret, h_ctx);
			if(p) {
				p->m_flag0 &= (aclspv_wrd_t)~aclcur_flag0_reg(ACLCUR_FLAG0_REG_SCOPED);
				p->m_flag0 |= aclcur_flag0_reg(ACLCUR_FLAG0_REG_ALLOCATED);
			}
		}

		/** seed the result so the loop header has a forward-edge value
		 * to build its carry phi from; without it the header load
		 * cannot resolve and the post-loop read would fall back to a
		 * value defined in the continue block (dominance violation). */
		if(c_has_ret) {
			aclspv_id_t id_nil;
			ae2f_ifezerr(id_nil = aclval_cfg_nil(ty_ret, h_ctx)) return 0;
			ae2f_ifezerr(aclcur_store_reg(id_ret, id_nil, h_ctx)) return 0;
		}
	}

	for(i = 0; i < c_num; ++i) {
		aclty_t* p_arg;
		aclty_id_t ty_arg;
		const aclty_t* p_ty;
		CXCursor c_parm;
		aclspv_id_t id_reg;

		ae2f_ifezerr(p_arg = aclty_get_fn_arg(p_fn, i, h_ctx)) return 0;
		ty_arg = p_arg->m_info.m_fn_arg.m_prm_id;
		p_ty = aclty_find(ty_arg, h_ctx);

		if(c_has_this && i == 0) {
			ae2f_ifezerr(id_reg = aclcur_reg_ghost(c_args[i], h_ctx)) return 0;
			{
				h_aclcur_t p = aclcur_get_node(id_reg, h_ctx);
				if(!p) return 0;
				p->m_ty = ty_arg; p->m_cur = clang_getNullCursor();
			}
			h_ctx->m_this = id_reg;
			continue;
		}
		/** the body being carved refers to ITS OWN parameter cursors. */
		c_parm = clang_Cursor_getArgument(c_fn, (unsigned)(c_has_this ? i - 1 : i));
		if(clang_Cursor_isNull(c_parm)) return 0;
		if(p_ty && p_ty->m_class == ACLTY_PTR) {
			ae2f_ifezerr(id_reg = aclcur_reg_ghost(c_args[i], h_ctx)) return 0;
			{
				h_aclcur_t p = aclcur_get_node(id_reg, h_ctx);
				if(!p) return 0;
				p->m_ty = ty_arg; p->m_cur = c_parm;
			}
			continue;
		}
		{
			const aclcur_reg_t REGIN = aclcur_regvar_in();
			ae2f_ifezerr(id_reg = aclcur_cfg_reg(c_parm, ty_arg,
						SpvStorageClassFunction, &REGIN, h_ctx)) return 0;
			ae2f_ifezerr(aclcur_store_reg(id_reg, c_args[i], h_ctx)) return 0;
			if(!aclemit_dtor_reserve(c_parm, id_reg, ty_arg, h_ctx)) return 0;
		}
	}

	DAT.m_ctx = h_ctx; DAT.m_vid = 0; DAT.m_ty = 0;
	old_active = h_ctx->m_inline.m_active;
	old_end = h_ctx->m_inline.m_lbl_end;
	old_ret = h_ctx->m_inline.m_reg_ret;
	old_ty = h_ctx->m_inline.m_ty_ret;
	old_base = h_ctx->m_inline.m_dtor_base;
	old_branched = h_ctx->m_inline.m_branched;
	old_depth = h_ctx->m_inline.m_depth;
	old_retflag = h_ctx->m_inline.m_reg_returned;
	old_merge = h_ctx->m_inline.m_merge_depth;
	old_terminated = h_ctx->m_inline.m_terminated;
	old_fnterm = aclcur_lbl_is_fnterm(h_ctx);
	old_callee = s_body_fn;

	h_ctx->m_inline.m_active = 1;
	h_ctx->m_inline.m_lbl_end = id_cont;
	h_ctx->m_inline.m_reg_ret = id_ret;
	h_ctx->m_inline.m_ty_ret = ty_ret;
	h_ctx->m_inline.m_dtor_base = c_dtor_base;
	h_ctx->m_inline.m_branched = 0;
	h_ctx->m_inline.m_depth = old_depth + 1;
	h_ctx->m_inline.m_reg_returned = 0;
	h_ctx->m_inline.m_merge_depth = 0;
	h_ctx->m_inline.m_terminated = 0;
	s_body_fn = c_fn;

	if(c_has_ret) {
		aclspv_id_t c_cond;
		if(!aclcur_cfg_jmp(id_header, h_ctx)
				|| !aclcur_cfg_lbl(id_header, h_ctx)
				|| !aclcur_cfg_lbl_loop(id_header, h_ctx)) {
			h_ctx->m_inline.m_active = old_active;
			return 0;
		}
		ae2f_ifezerr(c_cond = aclcur_load_reg(id_flag, h_ctx, 0)) {
			h_ctx->m_inline.m_active = old_active;
			return 0;
		}
		if(ty_ret != ACLSPV_ID_VOID) {
			ae2f_ifezerr(aclcur_load_reg(id_ret, h_ctx, 0)) {
				h_ctx->m_inline.m_active = old_active;
				return 0;
			}
		}
		if(!aclcur_cfg_jc(c_cond, id_body, id_end, 0, 0, id_end, id_cont, 0, h_ctx)
				|| !aclcur_cfg_lbl(id_body, h_ctx)) {
			h_ctx->m_inline.m_active = old_active;
			return 0;
		}
		if(!aclcur_store_reg(id_flag, aclval_cfg_bool(0, h_ctx), h_ctx)) {
			h_ctx->m_inline.m_active = old_active;
			return 0;
		}
	}

	if(!s_park_expr(h_ctx)) { h_ctx->m_inline.m_active = old_active; return 0; }

	clang_visitChildren(c_body, aclemit_body, &DAT);
	aclctx_err_check(h_ctx) { s_unpark_expr(h_ctx); h_ctx->m_inline.m_active = old_active; return 0; }
	if(!aclemit_dtor_flush_to(h_ctx, c_dtor_base)) { s_unpark_expr(h_ctx); return 0; }

	if(c_has_ret) {
		/** the body may have ended with a return (its block already branches
		 * to the continue target) or with an open block to close. never both:
		 * the exit block takes one terminator. */
		if(!h_ctx->m_inline.m_terminated
				&& aclcur_lbl_cur_is_open(h_ctx)) {
			const aclspv_id_t id_cur = aclcur_get_lbl_cur(h_ctx);

			/** a structurally emitted merge block nothing reaches (a
			 * switch whose every case returned) must be terminated too,
			 * but an edge from it would add a dead predecessor the
			 * continue block's phis do not carry. */
			if(id_cur && aclcur_ssa_lbl_live(id_cur, h_ctx)) {
				ae2f_ifezerr(aclcur_cfg_jmp(id_cont, h_ctx)) { s_unpark_expr(h_ctx); return 0; }
			} else {
				ae2f_ifezerr(aclcur_cfg_unreachable(h_ctx)) { s_unpark_expr(h_ctx); return 0; }
			}
		}
		ae2f_ifezerr(aclcur_cfg_lbl(id_cont, h_ctx)
				&& aclcur_cfg_jmp(id_header, h_ctx)
				&& aclcur_cfg_lbl(id_end, h_ctx)) { s_unpark_expr(h_ctx); return 0; }
	}
	s_unpark_expr(h_ctx);

	h_ctx->m_inline.m_active = old_active;
	h_ctx->m_inline.m_lbl_end = old_end;
	h_ctx->m_inline.m_reg_ret = old_ret;
	h_ctx->m_inline.m_ty_ret = old_ty;
	h_ctx->m_inline.m_dtor_base = old_base;
	h_ctx->m_inline.m_branched = old_branched;
	h_ctx->m_inline.m_depth = old_depth;
	h_ctx->m_inline.m_reg_returned = old_retflag;
	h_ctx->m_inline.m_merge_depth = old_merge;
	h_ctx->m_inline.m_terminated = old_terminated;
	aclcur_set_lbl_fnterm(old_fnterm, h_ctx);
	s_body_fn = old_callee;
	h_ctx->m_this = id_this;
	aclcur_set_fnret(c_fnret, h_ctx);
	aclcur_set_break_loop(id_brk, h_ctx);
	aclcur_set_continue_loop(id_cnt, h_ctx);

	if(wr_ret) {
		if(ty_ret != ACLSPV_ID_VOID) {
			aclcur_reg_untrack(id_ret, h_ctx);
			ae2f_ifezerr(*wr_ret = aclcur_load_reg(id_ret, h_ctx, 0)) return 0;
		} else *wr_ret = 0;
	}
	return 1;
}
