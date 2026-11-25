#include "./inline.h"

#include <aclctx/err.h>

#include <aclemit/body.h>
#include <aclemit/dtor.h>

#include <aclsym/fn.h>

#include <aclcur/common.h>
#include <aclcur/reg.h>
#include <aclcur/reg/var.h>

#include <aclty.h>
#include <aclty/fn/arg.h>

#include <aclutil/fn.h>
#include <aclutil/scale.h>

#include <util/log.h>

static CXCursor s_body_of(CXCursor c_fn);

struct s_body_pick {
	CXCursor	m_body;
};

static enum CXChildVisitResult s_body_pick_visit(
		CXCursor		c,
		ae2f_unused CXCursor	p,
		CXClientData		rw
		) {
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


struct s_ret_pick {
	aclspv_bool_t	m_has;
};

static enum CXChildVisitResult s_ret_pick_visit(
		CXCursor		c,
		ae2f_unused CXCursor	p,
		CXClientData		rw
		) {
	struct s_ret_pick* ae2f_restrict R = rw;

	if(clang_getCursorKind(c) == CXCursor_ReturnStmt) {
		R->m_has = 1;
		return CXChildVisit_Break;
	}

	return CXChildVisit_Recurse;
}

static aclspv_bool_t s_has_return(CXCursor c_body) {
	struct s_ret_pick R;

	R.m_has = 0;
	clang_visitChildren(c_body, s_ret_pick_visit, &R);
	return R.m_has;
}


struct s_body_simple {
	aclspv_wrd_t	m_n;
	aclspv_bool_t	m_ret;
};

static enum CXChildVisitResult s_body_simple_visit(
		CXCursor		c,
		ae2f_unused CXCursor	p,
		CXClientData		rw
		) {
	struct s_body_simple* ae2f_restrict B = rw;

	++B->m_n;

	if(clang_getCursorKind(c) == CXCursor_ReturnStmt)
		B->m_ret = 1;

	return CXChildVisit_Continue;
}

/** the body is exactly one return statement: inline it without an end label. */
static aclspv_bool_t s_body_simple_return(CXCursor c_body) {
	struct s_body_simple B;

	B.m_n = 0;
	B.m_ret = 0;
	clang_visitChildren(c_body, s_body_simple_visit, &B);
	return (aclspv_bool_t)(B.m_n == 1 && B.m_ret);
}

aclspv_bool_t aclemit_inline_fn(
		CXCursor	c_fn,
		CXCursor	c_body,
		const aclspv_id_t* c_args,
		aclspv_wrd_t	c_num,
		aclspv_id_t*	wr_ret,
		h_aclspv_ctx_t	h_ctx
		) {
	aclty_t* ae2f_restrict	p_fn;
	aclspv_id_t		id_fn_ty;
	aclspv_wrd_t		i;
	aclspv_id_t		id_end;
	aclspv_id_t		id_ret = 0;
	aclty_id_t		ty_ret;
	aclspv_wrd_t		c_dtor_base;
	aclspv_bool_t		c_has_this;
	aclspv_bool_t		c_has_ret;
	aclspv_bool_t		c_simple_ret;
	aclspv_bool_t		old_active, old_branched;
	aclspv_id_t		old_end, old_ret;
	aclspv_id_t		old_ty;
	aclspv_wrd_t		old_base, old_depth;
	aclspv_bool_t		c_fnret;
	aclspv_id_t		id_brk, id_cnt, id_this;
	aclemit_body_t		DAT;
	h_scale_t		S;

	ae2f_ifezerr(h_ctx)	return 0;

	if(h_ctx->m_inline.m_depth >= 16) {
		_aclctx_err(h_ctx, ACLSPV_COMPILE_UNHANDLED);
		return 0;
	}

	if(clang_Cursor_isNull(c_body))
		c_body = s_body_of(c_fn);

	unless(!clang_Cursor_isNull(c_body)) {
		_aclctx_err(h_ctx, ACLSPV_COMPILE_UNHANDLED);
		return 0;
	}

	ae2f_ifezerr(id_fn_ty = aclty_mk_fn1_this(
				c_fn, clang_getCursorType(c_fn), h_ctx))
		return 0;

	ae2f_ifezerr(p_fn = aclty_find(id_fn_ty, h_ctx))
		return 0;

	if(p_fn->m_info.m_fn.m_num_arg != c_num) {
		_aclctx_err(h_ctx, ACLSPV_COMPILE_UNHANDLED);
		return 0;
	}

	ty_ret		= p_fn->m_info.m_fn.m_ret_id;
	c_has_this	= aclutil_fn_has_this(c_fn);
	c_has_ret	= s_has_return(c_body);
	c_simple_ret	= c_has_ret ? s_body_simple_return(c_body) : 0;

	c_fnret	= aclcur_get_fnret(h_ctx);
	id_brk	= aclcur_get_break_loop(h_ctx);
	id_cnt	= aclcur_get_continue_loop(h_ctx);
	id_this	= h_ctx->m_this;

	S = get_last_scale_from_vec(&h_ctx->m_dtor_scope);
	c_dtor_base = S ? S->m_id : 0;

	if(!aclemit_dtor_push(h_ctx))
		return 0;

	id_end = (c_has_ret && !c_simple_ret) ? h_ctx->m_id++ : 0;

	if(ty_ret != ACLSPV_ID_VOID) {
		const aclcur_reg_t REGIN = aclcur_regvar_in();

		ae2f_ifezerr(id_ret = aclcur_cfg_reg(
					clang_getNullCursor()
					, ty_ret
					, SpvStorageClassFunction
					, &REGIN
					, h_ctx)) {
			_aclctx_err(h_ctx, ACLSPV_COMPILE_UNHANDLED);
			return 0;
		}
	}

	/** bind parameters to caller-provided values. */
	for(i = 0; i < c_num; ++i) {
		aclty_t* ae2f_restrict p_arg;
		aclty_id_t		ty_arg;
		const aclty_t* ae2f_restrict p_ty;
		CXCursor		c_parm;
		aclspv_id_t		id_reg;

		ae2f_ifezerr(p_arg = aclty_get_fn_arg(p_fn, i, h_ctx))
			return 0;

		ty_arg = p_arg->m_info.m_fn_arg.m_prm_id;
		p_ty = aclty_find(ty_arg, h_ctx);

		if(c_has_this && i == 0) {
			ae2f_ifezerr(id_reg = aclcur_cfg_reg(
						clang_getNullCursor()
						, ty_arg
						, SpvStorageClassFunction
						, 0
						, h_ctx))
				return 0;

			{
				h_aclcur_t p = aclcur_get_node(id_reg, h_ctx);

				if(!p)
					return 0;

				p->m_val = c_args[i];
				p->m_flag0 |= aclcur_flag0(ACLCUR_FLAG0_VALUE)
					| aclcur_flag0_reg(ACLCUR_FLAG0_REG_PARAM);
			}

			h_ctx->m_this = id_reg;
			continue;
		}

		c_parm = clang_Cursor_getArgument(
				c_fn, (unsigned)(c_has_this ? i - 1 : i));

		if(clang_Cursor_isNull(c_parm)) {
			_aclctx_err(h_ctx, ACLSPV_COMPILE_UNHANDLED);
			return 0;
		}

		if(p_ty && p_ty->m_class == ACLTY_PTR) {
			ae2f_ifezerr(id_reg = aclcur_cfg_reg(
						clang_getNullCursor()
						, ty_arg
						, SpvStorageClassFunction
						, 0
						, h_ctx))
				return 0;

			{
				h_aclcur_t p = aclcur_get_node(id_reg, h_ctx);

				if(!p)
					return 0;

				p->m_cur = c_parm;
				p->m_id = c_args[i];
				p->m_val = c_args[i];
				p->m_flag0 |= aclcur_flag0(ACLCUR_FLAG0_VALUE)
					| aclcur_flag0_reg(ACLCUR_FLAG0_REG_PARAM);
			}

			continue;
		}

		{
			const aclcur_reg_t REGIN = aclcur_regvar_in();

			ae2f_ifezerr(id_reg = aclcur_cfg_reg(
						c_parm
						, ty_arg
						, SpvStorageClassFunction
						, &REGIN
						, h_ctx))
				return 0;

			ae2f_ifezerr(aclcur_store_reg(id_reg, c_args[i], h_ctx))
				return 0;

			if(!aclemit_dtor_reserve(c_parm, id_reg, ty_arg, h_ctx))
				return 0;
		}
	}

	DAT.m_ctx	= h_ctx;
	DAT.m_vid	= 0;
	DAT.m_ty	= 0;

	old_active	= h_ctx->m_inline.m_active;
	old_end		= h_ctx->m_inline.m_lbl_end;
	old_ret		= h_ctx->m_inline.m_reg_ret;
	old_ty		= h_ctx->m_inline.m_ty_ret;
	old_base	= h_ctx->m_inline.m_dtor_base;
	old_branched	= h_ctx->m_inline.m_branched;
	old_depth	= h_ctx->m_inline.m_depth;

	h_ctx->m_inline.m_active	= 1;
	h_ctx->m_inline.m_lbl_end	= id_end;
	h_ctx->m_inline.m_reg_ret	= id_ret;
	h_ctx->m_inline.m_ty_ret	= ty_ret;
	h_ctx->m_inline.m_dtor_base	= c_dtor_base;
	h_ctx->m_inline.m_branched	= 0;
	h_ctx->m_inline.m_depth		= old_depth + 1;

	clang_visitChildren(c_body, aclemit_body, &DAT);

	aclctx_err_check(h_ctx) {
		h_ctx->m_inline.m_active = 0;
		return 0;
	}

	if(c_simple_ret) {
		/** the return path already flushed the inline scope. */
	}
	else if(!c_has_ret) {
		/** straight-line body: no end label, no synthetic branch. */
		if(!aclemit_dtor_flush_to(h_ctx, c_dtor_base))
			return 0;
	}
	else {
		if(!h_ctx->m_inline.m_branched) {
			if(!aclemit_dtor_flush_to(h_ctx, c_dtor_base))
				return 0;

			ae2f_ifnezerr(aclcur_cfg_jmp(id_end, h_ctx))
				return 0;
		}

		ae2f_ifnezerr(aclcur_cfg_lbl(id_end, h_ctx))
			return 0;
	}

	h_ctx->m_inline.m_active	= old_active;
	h_ctx->m_inline.m_lbl_end	= old_end;
	h_ctx->m_inline.m_reg_ret	= old_ret;
	h_ctx->m_inline.m_ty_ret	= old_ty;
	h_ctx->m_inline.m_dtor_base	= old_base;
	h_ctx->m_inline.m_branched	= old_branched;
	h_ctx->m_inline.m_depth		= old_depth;
	h_ctx->m_this = id_this;
	aclcur_set_fnret(c_fnret, h_ctx);
	aclcur_set_break_loop(id_brk, h_ctx);
	aclcur_set_continue_loop(id_cnt, h_ctx);

	if(wr_ret) {
		if(ty_ret != ACLSPV_ID_VOID) {
			ae2f_ifezerr(*wr_ret = aclcur_load_reg(id_ret, h_ctx, 0))
				return 0;
		}
		else *wr_ret = 0;
	}

	return 1;
}