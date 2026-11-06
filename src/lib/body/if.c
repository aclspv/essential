#ifndef	aclemit_body_if_cc
#define aclemit_body_if_cc

#include <aclattr/ifpart.h>
#include <aclattr/branchratio.h>

#include <aclexpr.h>
#include <aclcur/common.h>

#include <aclemit/body.h>
#include <aclutil/emitx.h>
#include <util/visitfromroot.h>

#include <ae2f/pat/dbg.h>
#include <ae2f/pat/macrofunc.h>

#include <.aclspv/util/mkname.h>

/** @brief it produces error on ctx. */
ae2f_MAC((L, )) aclemit_body_if(CXCursor h_cur, h_aclspv_ctx_t h_ctx) {

	struct s_attr_ifpart_t	L$$IFPART;
	aclspv_wrd_t		L$$IDVAL = 0,
				L$$WEIGHTS[2] = {0, 0},
				L$$IFCTL = SpvSelectionControlMaskNone;

	aclutil_literal		L$$LTLR;

	/** fnret merged across the branches: parent | true | false. */
	aclspv_bool_t		L$$FNRET_PAR = 0,
				L$$FNRET_TRUE = 0,
				L$$FNRET_FALSE = 0;

	ae2fpat_assertassume(h_ctx);

	L$$IFPART.m_n				= 0;
	L$$IFPART.m_n_dummy			= 0;
	L$$IFPART.m_b_true				= h_ctx->m_id++;
	L$$IFPART.m_b_false = L$$IFPART.m_b_default	= h_ctx->m_id++;
	L$$IFPART.m_ctx				= h_ctx;

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

	switch(aclexpr_eval(
				&L$$IDVAL
				, ACLSPV_ID_BOOL
				, L$$IFPART.m_parts[0]
				, &L$$LTLR
				, h_ctx))
	{
		case ACLEXPR_RET_NOT_THE_CASE:
		case ACLEXPR_RET_FAILURE:
		default:
			goto L$$LBL_END;

		case ACLEXPR_RET_SUCCESS:
		case ACLEXPR_RET_SUCCESS_CONSTANT:
			break;
		case ACLEXPR_RET_SUCCESS_LITERAL:
			if(L$$LTLR.m_api_int) {
				const enum CXChildVisitResult L$$RET = aclemit_entp_body(
						L$$IFPART.m_parts[1]
						, L$$IFPART.m_parts[1]
						, h_ctx
						);

				ae2f_unexpected_but_if(h_ctx->m_err)
					goto L$$LBL_END;

				if(L$$RET == CXChildVisit_Recurse) clang_visitChildren(
						L$$IFPART.m_parts[1]
						, aclemit_entp_body
						, h_ctx
						); ae2f_unexpected_but_if(h_ctx->m_err) goto L$$LBL_END;
			} else unless((L$$IFPART.m_n ^ 3) & 3) {
				const enum CXChildVisitResult L$$RET = aclemit_entp_body(
						L$$IFPART.m_parts[2]
						, L$$IFPART.m_parts[2]
						, h_ctx
						);

				ae2f_unexpected_but_if(h_ctx->m_err)
					goto L$$LBL_END;

				if(L$$RET == CXChildVisit_Recurse) clang_visitChildren(
						L$$IFPART.m_parts[2]
						, aclemit_entp_body
						, h_ctx
						); ae2f_unexpected_but_if(h_ctx->m_err) goto L$$LBL_END;
			}

			goto L$$LBL_END;
	}

	ae2f_expected_but_else(L$$IDVAL) {
		h_ctx->m_err = ACLSPV_COMPILE_IF_COND_INVAL;
		goto L$$LBL_END;
	}

	aclutil_visitfromroot(
			L$$IFPART.m_parts[1]
			, s_aclattr_bratio
			, &L$$WEIGHTS[0]
			);

	ae2f_expected_but_else(aclcur_cfg_jc(
				L$$IDVAL
				, L$$IFPART.m_b_true
				, L$$IFPART.m_b_false
				, L$$WEIGHTS[0]
				, L$$WEIGHTS[1]
				, L$$IFPART.m_b_default
				, (aclspv_id_t)0
				, L$$IFCTL	/** attribute */
				, h_ctx
				)) goto L$$LBL_END;

	{
		L$$FNRET_PAR = aclcur_get_fnret(h_ctx);
		L$$FNRET_TRUE = L$$FNRET_FALSE = 0;
		aclcur_set_fnret(0, h_ctx);

		ae2f_expected_but_else(aclcur_cfg_lbl(L$$IFPART.m_b_true, h_ctx))
			goto L$$LBL_END;

		aclutil_visitfromroot(L$$IFPART.m_parts[1]
				, aclemit_entp_body
				, h_ctx
				);

		ae2f_unexpected_but_if(h_ctx->m_err) goto L$$LBL_END;

		L$$FNRET_TRUE = aclcur_get_fnret(h_ctx);

		unless(L$$FNRET_TRUE) {
			ae2f_expected_but_else(aclcur_cfg_jmp(L$$IFPART.m_b_default, h_ctx))
				goto L$$LBL_END;
		}

		aclutil_mkname_dbg_fn(
				L$$IFPART.m_b_true
				, "if"
				, h_ctx
				, &h_ctx->m_tmp.m_v0
				);

		ae2f_ifnezerr(h_ctx->m_err) {
			goto L$$LBL_END;
		}
	}

	unless((L$$IFPART.m_n ^ 3) & 3) {
		aclcur_set_fnret(0, h_ctx);

		ae2f_expected_but_else(aclcur_cfg_lbl(L$$IFPART.m_b_false, h_ctx))
			goto L$$LBL_END;

		aclutil_visitfromroot(
				L$$IFPART.m_parts[2]
				, aclemit_entp_body
				, h_ctx
				);

		ae2f_ifnezerr(h_ctx->m_err)
			goto L$$LBL_END;

		L$$FNRET_FALSE = aclcur_get_fnret(h_ctx);

		unless(L$$FNRET_FALSE) {
			ae2f_expected_but_else(aclcur_cfg_jmp(L$$IFPART.m_b_default, h_ctx))
				goto L$$LBL_END;
		}

		aclutil_mkname_dbg_fn(
				L$$IFPART.m_b_false
				, "else"
				, h_ctx
				, &h_ctx->m_tmp.m_v0
				);
	}

	aclcur_set_fnret(L$$FNRET_PAR | L$$FNRET_TRUE | L$$FNRET_FALSE, h_ctx);

	aclutil_mkname_dbg_fn(
			L$$IFPART.m_b_default
			, "endif"
			, h_ctx
			, &h_ctx->m_tmp.m_v0
			);

	ae2f_expected_but_else(aclcur_cfg_lbl(L$$IFPART.m_b_default, h_ctx))
		goto L$$LBL_END;

	L$$LBL_END: (void)0;
} 


#endif
