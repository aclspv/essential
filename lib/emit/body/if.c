#include "./if.h"
#include "util/literal.h"

#include <attr/ifpart.h>
#include <clang-c/Index.h>
#include <emit/expr.h>
#include <emit/entp_body.h>

#include <util/emitx.h>
#include <util/cursor.h>

#include <ae2f/pat/dbg.h>


void aclemit_body_if(CXCursor h_cur, aclutil_fnctx *restrict FNCTX) {
#define	CTX		(FNCTX->m_ctx)

	struct s_attr_ifpart_t	IFPART;
	aclspv_id_t		IDVAL = 0;
	util_literal		LTLR;

	ae2fpat_assertassume(FNCTX);
	ae2fpat_assertassume(FNCTX->m_ctx);

	IFPART.m_n				= 0;
	IFPART.m_n_dummy			= 0;
	IFPART.m_b_true				= CTX->m_id++;
	IFPART.m_b_false = IFPART.m_b_default	= CTX->m_id++;
	IFPART.m_ctx				= CTX;

	clang_visitChildren(h_cur, s_attr_ifpart, &IFPART);
	dbg_prefix(a);
	dbg_call(fprintf, (stderr, "number ifpart: %u %u\n", IFPART.m_n, IFPART.m_n_dummy));


	dbg_prefix(a);
	dbg_call(fprintf, (stderr, "ifpart: %u %u %u\n", IFPART.m_b_true, IFPART.m_b_false, IFPART.m_b_default));

	switch(aclspv_get_expr(
				&IDVAL
				, ID_DEFAULT_BOOL
				, IFPART.m_parts[0]
				, &LTLR, CTX))
	{
		case EMIT_EXPR_NOT_THE_CASE:
		case EMIT_EXPR_FAILURE:
		default:
			return;

		case EMIT_EXPR_SUCCESS:
		case EMIT_EXPR_SUCCESS_CONSTANT:
			break;
		case EMIT_EXPR_SUCCESS_LITERAL:
			if(LTLR.m_api_int) {
				const enum CXChildVisitResult R = aclemit_entp_body(
						IFPART.m_parts[1]
						, IFPART.m_parts[1]
						, FNCTX
						);

				ae2f_unexpected_but_if(CTX->m_err)
					return;

				if(R == CXChildVisit_Recurse) clang_visitChildren(
						IFPART.m_parts[1]
						, aclemit_entp_body
						, FNCTX
						); ae2f_unexpected_but_if(CTX->m_err) return;
			} else unless((IFPART.m_n ^ 3) & 3) {
				const enum CXChildVisitResult R = aclemit_entp_body(
						IFPART.m_parts[2]
						, IFPART.m_parts[2]
						, FNCTX
						);

				ae2f_unexpected_but_if(CTX->m_err)
					return;

				if(R == CXChildVisit_Recurse) clang_visitChildren(
						IFPART.m_parts[2]
						, aclemit_entp_body
						, FNCTX
						); ae2f_unexpected_but_if(CTX->m_err) return;
			}

			return;
	}

	ae2f_expected_but_else(IDVAL) {
		CTX->m_err = ACLSPV_COMPILE_IF_COND_INVAL;
		return;
	}

	ae2f_unexpected_but_if(CTX->m_err = aclutil_tell_cursor_lbl(
				CTX->m_num_cursor
				, CTX->m_cursors.m_p
				, CTX)) return;

	ae2f_expected_but_else(CTX->m_count.m_fnimpl = util_emitx_3(
				&CTX->m_section.m_fnimpl
				, CTX->m_count.m_fnimpl
				, SpvOpSelectionMerge
				, IFPART.m_b_default
				, 0	/** attribute */
				)) return;

	ae2f_expected_but_else(CTX->m_count.m_fnimpl = util_emitx_4(
				&CTX->m_section.m_fnimpl
				, CTX->m_count.m_fnimpl
				, SpvOpBranchConditional
				, IDVAL
				, IFPART.m_b_true
				, IFPART.m_b_false
				)) return;

	{
		aclutil_fnctx		FNCTX_NEW = *FNCTX;
		enum CXChildVisitResult R;
		FNCTX_NEW.m_state0 = 0;

		ae2f_expected_but_else(CTX->m_count.m_fnimpl = util_emitx_2(
					&CTX->m_section.m_fnimpl
					, CTX->m_count.m_fnimpl
					, SpvOpLabel
					, IFPART.m_b_true
					)) return;

		FNCTX_NEW.m_lbl_snapshot = CTX->m_count.m_fnimpl;

		R = aclemit_entp_body(
				IFPART.m_parts[1]
				, IFPART.m_parts[1]
				, &FNCTX_NEW
				);

		ae2f_unexpected_but_if(CTX->m_err)
			return;

		if(R == CXChildVisit_Recurse) clang_visitChildren(
				IFPART.m_parts[1]
				, aclemit_entp_body
				, &FNCTX_NEW
				); ae2f_unexpected_but_if(CTX->m_err) return;

		FNCTX->m_state0		|= FNCTX_NEW.m_state0;

		ae2f_unexpected_but_if(CTX->m_err = aclutil_tell_cursor_lbl(
					CTX->m_num_cursor
					, CTX->m_cursors.m_p
					, CTX)) return;

		unless(FNCTX_NEW.m_state0 & ACLUTIL_FNCTX_STATE0_FNRET) {
			ae2f_expected_but_else(CTX->m_count.m_fnimpl = util_emitx_2(
						&CTX->m_section.m_fnimpl
						, CTX->m_count.m_fnimpl
						, SpvOpBranch
						, IFPART.m_b_default
						)) return;
		}
	}

	unless((IFPART.m_n ^ 3) & 3) {
		enum CXChildVisitResult R;
		aclutil_fnctx		FNCTX_NEW = *FNCTX;
		FNCTX_NEW.m_state0 = 0;

		ae2f_ifezerr(CTX->m_count.m_fnimpl = util_emitx_2(
					&CTX->m_section.m_fnimpl
					, CTX->m_count.m_fnimpl
					, SpvOpLabel
					, IFPART.m_b_false
					)) return;

		FNCTX_NEW.m_lbl_snapshot = CTX->m_count.m_fnimpl;

		R = aclemit_entp_body(
				IFPART.m_parts[2]
				, IFPART.m_parts[2]
				, &FNCTX_NEW
				);

		ae2f_unexpected_but_if(CTX->m_err)
			return;

		if(R == CXChildVisit_Recurse) clang_visitChildren(
				IFPART.m_parts[2]
				, aclemit_entp_body
				, &FNCTX_NEW
				); ae2f_unexpected_but_if(CTX->m_err) return;

		FNCTX->m_state0		|= FNCTX_NEW.m_state0;

		ae2f_unexpected_but_if(CTX->m_err = aclutil_tell_cursor_lbl(
					CTX->m_num_cursor
					, CTX->m_cursors.m_p
					, CTX)) return;

		unless(FNCTX_NEW.m_state0 & ACLUTIL_FNCTX_STATE0_FNRET) {
			ae2f_expected_but_else(CTX->m_count.m_fnimpl = util_emitx_2(
						&CTX->m_section.m_fnimpl
						, CTX->m_count.m_fnimpl
						, SpvOpBranch
						, IFPART.m_b_default
						)) return;
		}
	}

	ae2f_expected_but_else(CTX->m_count.m_fnimpl = util_emitx_2(
				&CTX->m_section.m_fnimpl
				, CTX->m_count.m_fnimpl
				, SpvOpLabel
				, IFPART.m_b_default
				)) return;
}
