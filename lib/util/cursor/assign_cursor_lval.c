#include <util/emitx.h>
#include <util/cursor.h>
#include <aclexpr.h>
#include <ae2f/bll.h>
#include <ae2f/cgoto.h>
#include <.aclspv/util/mkname.h>

ae2f_inline_f ae2f_ccconst static aclspv_bool_t s_is_lval(const enum CXCursorKind	c_cursorkind) {
	switch((uint_leastenum_t)c_cursorkind) {
		default: return 0;
		case CXCursor_VariableRef:
		case CXCursor_VarDecl:
			return 1;
	}
}

aclspv_id_t aclutil_assign_cursor_lval(
		aclutil_cursor* ae2f_restrict const wr_cursor,
		const h_aclspv_ctx_t			h_ctx,
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
	

	unless(s_is_lval(wr_cursor->m_cursor.kind)) {
		err_prefix(a);
		err_call(fprintf, (stderr, "wr_cursor is not left-value %d\n", (int)wr_cursor->m_cursor.kind));
		h_ctx->m_err = ACLSPV_COMPILE_ASSIGN_ON_NON_LVAL;
		return 0; 
	}

	if(r_literal_opt) *r_literal_opt = (*wr_cursor).m_data.m_vardecl.m_literal;
	if(rd_literal_to_assign && (c_masklit & ACLUTIL_LITERAL_LITERAL)) {
		(*wr_cursor).m_data.m_vardecl.m_mask |= 3 | ACLUTIL_CURSOR_VAR_PREDICTABLE;

		switch(aclexpr_arithmetic_cast_literal(
					c_val_ty
					, *rd_literal_to_assign
					, &(*wr_cursor).m_data.m_vardecl.m_init_val
					, (*wr_cursor).m_data.m_vardecl.m_type_id
					, &(*wr_cursor).m_data.m_vardecl.m_literal
					, h_ctx))
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

				goto LBL_RETNOLBL;
		}
	}

	if(c_val) {
		dbg_puts("[assign] c_val");
		(*wr_cursor).m_data.m_vardecl.m_mask	|= ACLUTIL_CURSOR_VAR_PREDICTABLE;
		(*wr_cursor).m_data.m_vardecl.m_mask	&= ~((aclspv_wrd_t)ACLUTIL_CURSOR_VAR_LITERAL);
		switch(aclexpr_arithmetic_cast_non_literal(
					c_val
					, c_val_ty
					, c_masklit
					, &(*wr_cursor).m_data.m_vardecl.m_init_val
					, (*wr_cursor).m_data.m_vardecl.m_type_id
					, h_ctx
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
				ae2f_ifezerr(wr_cursor->m_data.m_vardecl.m_init_val)
					h_ctx->m_err = ACLSPV_COMPILE_ASSIGN_CAST_GOT_ZERO;
				return (*wr_cursor).m_data.m_vardecl.m_init_val;
		}
	}

	if((*wr_cursor).m_data.m_vardecl.m_mask & ACLUTIL_CURSOR_VAR_PREDICTABLE) {
		goto LBL_RETNOLBL;
	}

	(*wr_cursor).m_data.m_vardecl.m_mask	 |= ACLUTIL_CURSOR_VAR_PREDICTABLE;
	ae2f_expected_but_else(h_ctx->m_count.m_fnimpl = aclutil_emitx_4(
				&h_ctx->m_section.m_fnimpl
				, h_ctx->m_count.m_fnimpl
				, SpvOpLoad
				, (*wr_cursor).m_data.m_vardecl.m_type_id
				, (*wr_cursor).m_data.m_vardecl.m_init_val = h_ctx->m_id++
				, (*wr_cursor).m_data.m_vardecl.m_id)) {
		h_ctx->m_err = ACLSPV_COMPILE_ALLOC_FAILED_CURSOR;
		return 0;
	}

	if(h_ctx->m_flag0 & ACLSPV_CTX_FLAG0_SYMBOL) {
		CXString SPL = clang_getCursorSpelling((wr_cursor)->m_cursor);

		if(SPL.data) {
			_aclutil_mkname_dbg(L, wr_cursor->m_data.m_vardecl.m_init_val
					, SPL.data
					, *(h_ctx)
					, (h_ctx)->m_tmp.m_v0
					);
		}

		clang_disposeString(SPL);
	}

LBL_RETNOLBL:

	return (*wr_cursor).m_data.m_vardecl.m_init_val;
}
