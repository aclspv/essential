#ifndef	aclemit_body_if_cc
#define aclemit_body_if_cc

#include <attr/ifpart.h>
#include <attr/branchratio.h>

#include <clang-c/Index.h>
#include <emit/expr.h>
#include <emit/entp_body.h>

#include <spirv/unified1/spirv.h>
#include <util/emitx.h>
#include <util/cursor.h>
#include <util/fnctx.h>
#include <util/visitfromroot.h>

#include <ae2f/pat/dbg.h>
#include <ae2f/pat/macrofunc.h>

#include <.aclspv/util/mkname.h>

/** @brief it produces error on ctx. */
ae2f_MAC((L, )) aclemit_body_if(CXCursor h_cur, aclutil_fnctx * ae2f_restrict h_fnctx) {

	struct s_attr_ifpart_t	L$$IFPART;
	aclspv_wrd_t		L$$IDVAL = 0,
				L$$WEIGHTS[2] = {0, 0},
				L$$IFCTL = SpvSelectionControlMaskNone;

	util_literal		L$$LTLR;

	ae2fpat_assertassume(h_fnctx);
	ae2fpat_assertassume(h_fnctx->m_ctx);

	L$$IFPART.m_n				= 0;
	L$$IFPART.m_n_dummy			= 0;
	L$$IFPART.m_b_true				= (h_fnctx->m_ctx)->m_id++;
	L$$IFPART.m_b_false = L$$IFPART.m_b_default	= (h_fnctx->m_ctx)->m_id++;
	L$$IFPART.m_ctx				= (h_fnctx->m_ctx);

	clang_visitChildren(h_cur, s_attr_ifpart, &L$$IFPART);

	dbg_prefix(a);
	dbg_call(fprintf, (stderr, "number ifpart: %u %u\n", L$$IFPART.m_n, L$$IFPART.m_n_dummy));


	dbg_prefix(a);
	dbg_call(fprintf, (stderr
				, "ifpart: %u %u %u\n"
				, L$$IFPART.m_b_true
				, L$$IFPART.m_b_false
				, L$$IFPART.m_b_default)
			);

	switch(aclspv_get_expr(
				&L$$IDVAL
				, ID_DEFAULT_BOOL
				, L$$IFPART.m_parts[0]
				, &L$$LTLR, (h_fnctx->m_ctx)))
	{
		case EMIT_EXPR_NOT_THE_CASE:
		case EMIT_EXPR_FAILURE:
		default:
			goto L$$LBL_END;

		case EMIT_EXPR_SUCCESS:
		case EMIT_EXPR_SUCCESS_CONSTANT:
			break;
		case EMIT_EXPR_SUCCESS_LITERAL:
			if(L$$LTLR.m_api_int) {
				const enum CXChildVisitResult L$$RET = aclemit_entp_body(
						L$$IFPART.m_parts[1]
						, L$$IFPART.m_parts[1]
						, h_fnctx
						);

				ae2f_unexpected_but_if((h_fnctx->m_ctx)->m_err)
					goto L$$LBL_END;

				if(L$$RET == CXChildVisit_Recurse) clang_visitChildren(
						L$$IFPART.m_parts[1]
						, aclemit_entp_body
						, h_fnctx
						); ae2f_unexpected_but_if((h_fnctx->m_ctx)->m_err) goto L$$LBL_END;
			} else unless((L$$IFPART.m_n ^ 3) & 3) {
				const enum CXChildVisitResult L$$RET = aclemit_entp_body(
						L$$IFPART.m_parts[2]
						, L$$IFPART.m_parts[2]
						, h_fnctx
						);

				ae2f_unexpected_but_if((h_fnctx->m_ctx)->m_err)
					goto L$$LBL_END;

				if(L$$RET == CXChildVisit_Recurse) clang_visitChildren(
						L$$IFPART.m_parts[2]
						, aclemit_entp_body
						, h_fnctx
						); ae2f_unexpected_but_if((h_fnctx->m_ctx)->m_err) goto L$$LBL_END;
			}

			goto L$$LBL_END;
	}

	ae2f_expected_but_else(L$$IDVAL) {
		(h_fnctx->m_ctx)->m_err = ACLSPV_COMPILE_IF_COND_INVAL;
		goto L$$LBL_END;
	}

	aclutil_visitfromroot(
			L$$IFPART.m_parts[1]
			, s_aclattr_bratio
			, &L$$WEIGHTS[0]
			);

	ae2f_unexpected_but_if((h_fnctx->m_ctx)->m_err = aclutil_tell_cursor_lbl(
				(h_fnctx->m_ctx)->m_num_cursor
				, (h_fnctx->m_ctx)->m_cursors.m_p
				, (h_fnctx->m_ctx)
				, h_fnctx->m_var_stage + 1)) goto L$$LBL_END;

	ae2f_expected_but_else((h_fnctx->m_ctx)->m_count.m_fnimpl = util_emitx_3(
				&(h_fnctx->m_ctx)->m_section.m_fnimpl
				, (h_fnctx->m_ctx)->m_count.m_fnimpl
				, SpvOpSelectionMerge
				, L$$IFPART.m_b_default
				, L$$IFCTL	/** attribute */
				)) goto L$$LBL_END;

	if(L$$WEIGHTS[0] || L$$WEIGHTS[1]) {
		ae2f_expected_but_else((h_fnctx->m_ctx)->m_count.m_fnimpl = util_emitx_6(
					&(h_fnctx->m_ctx)->m_section.m_fnimpl
					, (h_fnctx->m_ctx)->m_count.m_fnimpl
					, SpvOpBranchConditional
					, L$$IDVAL
					, L$$IFPART.m_b_true
					, L$$IFPART.m_b_false
					, L$$WEIGHTS[0]
					, L$$WEIGHTS[1]
					)) goto L$$LBL_END;
	} else {
		ae2f_expected_but_else((h_fnctx->m_ctx)->m_count.m_fnimpl = util_emitx_4(
					&(h_fnctx->m_ctx)->m_section.m_fnimpl
					, (h_fnctx->m_ctx)->m_count.m_fnimpl
					, SpvOpBranchConditional
					, L$$IDVAL
					, L$$IFPART.m_b_true
					, L$$IFPART.m_b_false
					)) goto L$$LBL_END;
	}

	{
		aclutil_fnctx		L$$FNCTX_NEW = *h_fnctx;
		L$$FNCTX_NEW.m_state0 = 0;
		++L$$FNCTX_NEW.m_var_stage;

		ae2f_expected_but_else((h_fnctx->m_ctx)->m_count.m_fnimpl = util_emitx_2(
					&(h_fnctx->m_ctx)->m_section.m_fnimpl
					, (h_fnctx->m_ctx)->m_count.m_fnimpl
					, SpvOpLabel
					, L$$IFPART.m_b_true
					)) goto L$$LBL_END;

		L$$FNCTX_NEW.m_lbl_snapshot = (h_fnctx->m_ctx)->m_count.m_fnimpl;

		aclutil_visitfromroot(L$$IFPART.m_parts[1]
				, aclemit_entp_body
				, &L$$FNCTX_NEW
				);

		ae2f_unexpected_but_if((h_fnctx->m_ctx)->m_err) goto L$$LBL_END;

		h_fnctx->m_state0		|= L$$FNCTX_NEW.m_state0;

		ae2f_unexpected_but_if((h_fnctx->m_ctx)->m_err = aclutil_tell_cursor_lbl(
					(h_fnctx->m_ctx)->m_num_cursor
					, (h_fnctx->m_ctx)->m_cursors.m_p
					, (h_fnctx->m_ctx)
					, h_fnctx->m_var_stage + 1)) goto L$$LBL_END;

		unless(L$$FNCTX_NEW.m_state0 & ACLUTIL_FNCTX_STATE0_FNRET) {
			ae2f_expected_but_else((h_fnctx->m_ctx)->m_count.m_fnimpl = util_emitx_2(
						&(h_fnctx->m_ctx)->m_section.m_fnimpl
						, (h_fnctx->m_ctx)->m_count.m_fnimpl
						, SpvOpBranch
						, L$$IFPART.m_b_default
						)) goto L$$LBL_END;
		}

		aclutil_mkname_dbg_fn(
				L$$IFPART.m_b_true
				, "if"
				, (h_fnctx)->m_ctx
				, &(h_fnctx)->m_ctx->m_tmp.m_v0
				);
		ae2f_ifnezerr(h_fnctx->m_ctx->m_err) {
			goto L$$LBL_END;
		}
	}

	unless((L$$IFPART.m_n ^ 3) & 3) {
		aclutil_fnctx		L$$FNCTX_NEW = *h_fnctx;
		L$$FNCTX_NEW.m_state0 = 0;

		ae2f_ifezerr((h_fnctx->m_ctx)->m_count.m_fnimpl = util_emitx_2(
					&(h_fnctx->m_ctx)->m_section.m_fnimpl
					, (h_fnctx->m_ctx)->m_count.m_fnimpl
					, SpvOpLabel
					, L$$IFPART.m_b_false
					)) goto L$$LBL_END;

		L$$FNCTX_NEW.m_lbl_snapshot = (h_fnctx->m_ctx)->m_count.m_fnimpl;
		++L$$FNCTX_NEW.m_var_stage;

		aclutil_visitfromroot(
				L$$IFPART.m_parts[2]
				, aclemit_entp_body
				, &L$$FNCTX_NEW
				);

		ae2f_ifnezerr(((h_fnctx)->m_ctx)->m_err)
			goto L$$LBL_END;

		h_fnctx->m_state0		|= L$$FNCTX_NEW.m_state0;

		ae2f_ifnezerr((h_fnctx->m_ctx)->m_err = aclutil_tell_cursor_lbl(
					(h_fnctx->m_ctx)->m_num_cursor
					, (h_fnctx->m_ctx)->m_cursors.m_p
					, (h_fnctx->m_ctx)
					, (h_fnctx)->m_var_stage + 1)) goto L$$LBL_END;

		unless(L$$FNCTX_NEW.m_state0 & ACLUTIL_FNCTX_STATE0_FNRET) {
			ae2f_expected_but_else((h_fnctx->m_ctx)->m_count.m_fnimpl = util_emitx_2(
						&(h_fnctx->m_ctx)->m_section.m_fnimpl
						, (h_fnctx->m_ctx)->m_count.m_fnimpl
						, SpvOpBranch
						, L$$IFPART.m_b_default
						)) goto L$$LBL_END;
		}

		aclutil_mkname_dbg_fn(
				L$$IFPART.m_b_false
				, "else"
				, (h_fnctx)->m_ctx
				, &(h_fnctx)->m_ctx->m_tmp.m_v0
				);
	}

	aclutil_mkname_dbg_fn(
			L$$IFPART.m_b_default
			, "endif"
			, (h_fnctx)->m_ctx
			, &(h_fnctx)->m_ctx->m_tmp.m_v0
			);

	ae2f_expected_but_else((h_fnctx->m_ctx)->m_count.m_fnimpl = util_emitx_2(
				&(h_fnctx->m_ctx)->m_section.m_fnimpl
				, (h_fnctx->m_ctx)->m_count.m_fnimpl
				, SpvOpLabel
				, L$$IFPART.m_b_default
				)) goto L$$LBL_END;

	L$$LBL_END: (void)0;
} 


#endif
