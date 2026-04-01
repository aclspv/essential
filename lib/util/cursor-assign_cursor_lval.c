#include "aclspv.h"
#include "util/log.h"
#include <util/emitx.h>
#include <util/cursor.h>
#include <emit/expr.h>

ae2f_inline ae2f_ccconst static aclspv_bool_t s_is_lval(const enum CXCursorKind	c_cursorkind) {
	switch((uint_fastenum_t)c_cursorkind) {
		default: return 0;
		case CXCursor_DeclRefExpr:
			return 1;
	}
}

aclspv_id_t aclutil_assign_cursor_lval(
		util_cursor* ae2f_restrict const wr_cursor,
		const h_util_ctx_t			h_ctx,
		const aclspv_id_t			c_val,
		const aclspv_id_t			c_val_ty,
		const B_util_literal_t			c_masklit,
		const util_literal* ae2f_restrict	rd_literal_to_assign,
		util_literal*	ae2f_restrict		r_literal_opt
		) {
	if(s_is_lval(wr_cursor->m_cursor.kind)) return 0;
	if(r_literal_opt) *r_literal_opt = (*wr_cursor).m_data.m_var_simple.m_literal;
	if(rd_literal_to_assign) {
		(*wr_cursor).m_data.m_var_simple.m_is_predictable	= 1;
		(*wr_cursor).m_data.m_var_simple.m_mask_literal
			= 3;

		switch(aclexpr_arithmetic_cast_literal(
					c_val_ty
					, *rd_literal_to_assign
					, &(*wr_cursor).m_data.m_var_simple.m_init_val
					, (*wr_cursor).m_data.m_var_simple.m_type_id
					, &(*wr_cursor).m_data.m_var_simple.m_literal
					, h_ctx))
		{
			case EMIT_EXPR_NOT_THE_CASE:
			case EMIT_EXPR_SUCCESS:
			case EMIT_EXPR_SUCCESS_CONSTANT:
			case EMIT_EXPR_FAILURE:
			default:
				return 0;

			case EMIT_EXPR_SUCCESS_LITERAL:
				if(r_literal_opt) *r_literal_opt
					= (*wr_cursor).m_data.m_var_simple.m_literal;

				dbg_prefix(a);
				dbg_call((void), printf("assign_cursor_lval: initval%u", wr_cursor->m_data.m_var_simple.m_init_val));
				return (*wr_cursor).m_data.m_var_simple.m_init_val;
		}
	}

	if(c_val) {
		(*wr_cursor).m_data.m_var_simple.m_is_predictable	= 1;
		switch(aclexpr_arithmetic_cast_non_literal(
					c_val
					, c_val_ty
					, c_masklit
					, &(*wr_cursor).m_data.m_var_simple.m_init_val
					, (*wr_cursor).m_data.m_var_simple.m_type_id
					, h_ctx
					)) {
			case EMIT_EXPR_SUCCESS_LITERAL:
			case EMIT_EXPR_FAILURE:
			case EMIT_EXPR_NOT_THE_CASE:
			default:
				return 0;

			case EMIT_EXPR_SUCCESS:
				(*wr_cursor).m_data.m_var_simple.m_mask_literal &= 0;
				ae2f_fallthrough;
			case EMIT_EXPR_SUCCESS_CONSTANT:
				(*wr_cursor).m_data.m_var_simple.m_mask_literal &= 1;
				return (*wr_cursor).m_data.m_var_simple.m_init_val;
		}
	}

	if((*wr_cursor).m_data.m_var_simple.m_is_predictable) {
		return (*wr_cursor).m_data.m_var_simple.m_init_val;
	}

	(*wr_cursor).m_data.m_var_simple.m_is_predictable	= 1;
	ae2f_expected_but_else(h_ctx->m_count.m_fnimpl = util_emitx_4(
				&h_ctx->m_section.m_fnimpl
				, h_ctx->m_count.m_fnimpl
				, SpvOpLoad
				, (*wr_cursor).m_data.m_var_simple.m_type_id
				, (*wr_cursor).m_data.m_var_simple.m_init_val = h_ctx->m_id++
				, (*wr_cursor).m_data.m_var_simple.m_id)) {
		h_ctx->m_err = ACLSPV_COMPILE_ALLOC_FAILED_CURSOR;
		return 0;
	}

	return (*wr_cursor).m_data.m_var_simple.m_init_val;
}
