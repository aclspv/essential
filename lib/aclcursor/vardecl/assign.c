#include <aclexpr.h>
#include <aclcursor.h>

#include <util/emitx.h>
#include <ae2f/bll.h>
#include <ae2f/cgoto.h>
#include <.aclspv/util/mkname.h>

aclspv_id_t aclcursor_vardecl_assign(
		x_aclcursor* ae2f_restrict const wr_cursor,
		const aclutil_fnctx* ae2f_restrict	h_fnctx,
		const aclspv_id_t			c_val,
		const aclspv_id_t			c_val_ty,
		const aclutil_ltrlmask_t			c_masklit,
		const aclutil_literal* ae2f_restrict	rd_literal_to_assign,
		aclutil_literal*	ae2f_restrict		r_literal_opt
		) {
	aclutil_literal	LIT_DUMMY;

	r_literal_opt = ae2f_bll_sel_ptrbool_naive1(aclutil_literal* ae2f_restrict, r_literal_opt, r_literal_opt, &LIT_DUMMY);

	dbg_prefix();
	dbg_call((void), fprintf(stderr, "assign_cursor_lval | typeid: %u typeid_req: %u\n"
				, c_val_ty
				, (*wr_cursor).m_data.m_vardecl.m_type_id));
	

	unless(aclcursor_is_vardecl(wr_cursor)) {
		err_prefix(a);
		err_call(fprintf, (stderr, "wr_cursor is not left-value %d\n", (int)wr_cursor->m_cursor.kind));
		h_fnctx->m_ctx->m_err = ACLSPV_COMPILE_ASSIGN_ON_NON_LVAL;
		return 0; 
	}

	if(r_literal_opt) *r_literal_opt = (*wr_cursor).m_data.m_vardecl.m_literal;
	if(rd_literal_to_assign && (c_masklit & ACLUTIL_LITERAL_LITERAL)) {
		(*wr_cursor).m_data.m_vardecl.m_mask |= 3 | ACLCURSOR_VARDECL_PREDICTABLE;

		switch(aclexpr_arithmetic_cast_literal(
					c_val_ty
					, *rd_literal_to_assign
					, &(*wr_cursor).m_data.m_vardecl.m_init_val
					, (*wr_cursor).m_data.m_vardecl.m_type_id
					, &(*wr_cursor).m_data.m_vardecl.m_literal
					, h_fnctx->m_ctx))
		{
			case ACLEXPR_RET_NOT_THE_CASE:
			case ACLEXPR_RET_SUCCESS:
			case ACLEXPR_RET_SUCCESS_CONSTANT:
			case ACLEXPR_RET_FAILURE:
			default:
				return 0;

			case ACLEXPR_RET_SUCCESS_LITERAL:
				*r_literal_opt = (*wr_cursor).m_data.m_vardecl.m_literal;

				dbg_prefix(a);
				dbg_call((void), fprintf(
							stderr
							, "assign_cursor_lval: initval %u\n"
							, wr_cursor->m_data.m_vardecl.m_init_val
							));

				return wr_cursor->m_data.m_vardecl.m_init_val;
		}
	}

	if(c_val) {
		dbg_prefix(a);
		dbg_call((void), fprintf(stderr, "[assign] c_val %u\n", (unsigned)c_val));
		(*wr_cursor).m_data.m_vardecl.m_mask	|= ACLCURSOR_VARDECL_PREDICTABLE;
		(*wr_cursor).m_data.m_vardecl.m_mask	&= ~((aclspv_wrd_t)ACLCURSOR_VARDECL_LITERAL);
		switch(aclexpr_arithmetic_cast_non_literal(
					c_val
					, c_val_ty
					, c_masklit
					, &(*wr_cursor).m_data.m_vardecl.m_init_val
					, (*wr_cursor).m_data.m_vardecl.m_type_id
					, h_fnctx->m_ctx
					)) {
			case ACLEXPR_RET_SUCCESS_LITERAL:
			case ACLEXPR_RET_FAILURE:
			case ACLEXPR_RET_NOT_THE_CASE:
			default:
				return 0;
			case ACLEXPR_RET_SUCCESS_CONSTANT:
				(*wr_cursor).m_data.m_vardecl.m_mask |= ACLUTIL_LITERAL_CONSTANT;
				ae2f_fallthrough;

			case ACLEXPR_RET_SUCCESS:
				dbg_prefix();
				dbg_call((void), fprintf(stderr, "[assign] c_val casted %u\n"
							, (unsigned)wr_cursor->m_data.m_vardecl.m_init_val
							));
				ae2f_ifezerr(wr_cursor->m_data.m_vardecl.m_init_val)
					h_fnctx->m_ctx->m_err = ACLSPV_COMPILE_ASSIGN_CAST_GOT_ZERO;
				return (*wr_cursor).m_data.m_vardecl.m_init_val;
		}
	}

	if((*wr_cursor).m_data.m_vardecl.m_mask & ACLCURSOR_VARDECL_PREDICTABLE) {
		return (*wr_cursor).m_data.m_vardecl.m_init_val;
	}

	wr_cursor->m_data.m_vardecl.m_mask	 |= ACLCURSOR_VARDECL_PREDICTABLE;
	wr_cursor->m_data.m_vardecl.m_mask &= ~(aclspv_wrd_t)ACLCURSOR_VARDECL_LITERAL;
	ae2f_expected_but_else(h_fnctx->m_ctx->m_count.m_fnimpl = aclutil_emitx_4(
				&h_fnctx->m_ctx->m_section.m_fnimpl
				, h_fnctx->m_ctx->m_count.m_fnimpl
				, SpvOpLoad
				, (*wr_cursor).m_data.m_vardecl.m_type_id
				, (*wr_cursor).m_data.m_vardecl.m_init_val = h_fnctx->m_ctx->m_id++
				, (*wr_cursor).m_data.m_vardecl.m_id)) {
		h_fnctx->m_ctx->m_err = ACLSPV_COMPILE_ALLOC_FAILED_CURSOR;
		return 0;
	}

	if(h_fnctx->m_ctx->m_flag0 & ACLSPV_CTX_FLAG0_SYMBOL) {
		CXString SPL = clang_getCursorSpelling((wr_cursor)->m_cursor);

		if(SPL.data) {
			_aclutil_mkname_dbg(L, wr_cursor->m_data.m_vardecl.m_init_val
					, SPL.data
					, *(h_fnctx->m_ctx)
					, (h_fnctx->m_ctx)->m_tmp.m_v0
					);
		}

		clang_disposeString(SPL);
	}

	return (*wr_cursor).m_data.m_vardecl.m_init_val;
}
