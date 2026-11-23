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
ae2f_MAC((L, )) aclemit_body_if(CXCursor h_cur, aclemit_body_t& rdwr_body) {
	struct s_attr_ifpart_t	L$$IFPART;
	aclspv_wrd_t		L$$IDVAL = 0,
				L$$WEIGHTS[2] = {0, 0},
				L$$IFCTL = SpvSelectionControlMaskNone;

	acltrl_t		L$$LTLR;

	/** fnret merged across the branches: parent | true | false. */
	aclspv_bool_t		L$$FNRET_PAR = 0,
				L$$FNRET_TRUE = 0,
				L$$FNRET_FALSE = 0;

	(rdwr_body).m_vid = 0;

	L$$IFPART.m_n				= 0;
	L$$IFPART.m_n_dummy			= 0;
	L$$IFPART.m_b_true				= aclctx_newid((rdwr_body).m_ctx);
	L$$IFPART.m_b_false = L$$IFPART.m_b_default	= aclctx_newid((rdwr_body).m_ctx);
	L$$IFPART.m_ctx				= (rdwr_body).m_ctx;

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
				, (rdwr_body).m_ctx))
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
				const enum CXChildVisitResult L$$RET = aclemit_body(
						L$$IFPART.m_parts[1]
						, L$$IFPART.m_parts[1]
						, &(rdwr_body)
						);

				ae2f_unexpected_but_if((rdwr_body).m_ctx->m_err)
					goto L$$LBL_END;

				if(L$$RET == CXChildVisit_Recurse) clang_visitChildren(
						L$$IFPART.m_parts[1]
						, aclemit_body
						, &(rdwr_body)
						); ae2f_unexpected_but_if((rdwr_body).m_ctx->m_err) goto L$$LBL_END;
			} else unless((L$$IFPART.m_n ^ 3) & 3) {
				const enum CXChildVisitResult L$$RET = aclemit_body(
						L$$IFPART.m_parts[2]
						, L$$IFPART.m_parts[2]
						, &(rdwr_body)
						);

				ae2f_unexpected_but_if((rdwr_body).m_ctx->m_err)
					goto L$$LBL_END;

				if(L$$RET == CXChildVisit_Recurse) clang_visitChildren(
						L$$IFPART.m_parts[2]
						, aclemit_body
						, &(rdwr_body)
						); ae2f_unexpected_but_if((rdwr_body).m_ctx->m_err) goto L$$LBL_END;
			}

			goto L$$LBL_END;
	}

	ae2f_expected_but_else(L$$IDVAL) {
		(rdwr_body).m_ctx->m_err = ACLSPV_COMPILE_IF_COND_INVAL;
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
				, (rdwr_body).m_ctx
				)) goto L$$LBL_END;

	{
		L$$FNRET_PAR = aclcur_get_fnret((rdwr_body).m_ctx);
		L$$FNRET_TRUE = L$$FNRET_FALSE = 0;
		aclcur_set_fnret(0, (rdwr_body).m_ctx);

		ae2f_expected_but_else(aclcur_cfg_lbl(L$$IFPART.m_b_true, (rdwr_body).m_ctx))
			goto L$$LBL_END;

		aclutil_visitfromroot(L$$IFPART.m_parts[1]
				, aclemit_body
				, &(rdwr_body)
				);

		ae2f_unexpected_but_if((rdwr_body).m_ctx->m_err) goto L$$LBL_END;

		L$$FNRET_TRUE = aclcur_get_fnret((rdwr_body).m_ctx);

		unless(L$$FNRET_TRUE) {
			ae2f_expected_but_else(aclcur_cfg_jmp(L$$IFPART.m_b_default, (rdwr_body).m_ctx))
				goto L$$LBL_END;
		}

		aclutil_mkname_dbg_fn(
				L$$IFPART.m_b_true
				, "if"
				, (rdwr_body).m_ctx
				, &(rdwr_body).m_ctx->m_tmp.m_v0
				);

		ae2f_ifnezerr((rdwr_body).m_ctx->m_err) {
			goto L$$LBL_END;
		}
	}

	unless((L$$IFPART.m_n ^ 3) & 3) {
		aclcur_set_fnret(0, (rdwr_body).m_ctx);

		ae2f_expected_but_else(aclcur_cfg_lbl(L$$IFPART.m_b_false, (rdwr_body).m_ctx))
			goto L$$LBL_END;

		aclutil_visitfromroot(
				L$$IFPART.m_parts[2]
				, aclemit_body
				, &(rdwr_body)
				);

		ae2f_ifnezerr((rdwr_body).m_ctx->m_err)
			goto L$$LBL_END;

		L$$FNRET_FALSE = aclcur_get_fnret((rdwr_body).m_ctx);

		unless(L$$FNRET_FALSE) {
			ae2f_expected_but_else(aclcur_cfg_jmp(L$$IFPART.m_b_default, (rdwr_body).m_ctx))
				goto L$$LBL_END;
		}

		aclutil_mkname_dbg_fn(
				L$$IFPART.m_b_false
				, "else"
				, (rdwr_body).m_ctx
				, &(rdwr_body).m_ctx->m_tmp.m_v0
				);
	}

	aclcur_set_fnret(L$$FNRET_PAR | L$$FNRET_TRUE | L$$FNRET_FALSE, (rdwr_body).m_ctx);

	aclutil_mkname_dbg_fn(
			L$$IFPART.m_b_default
			, "endif"
			, (rdwr_body).m_ctx
			, &(rdwr_body).m_ctx->m_tmp.m_v0
			);

	ae2f_expected_but_else(aclcur_cfg_lbl(L$$IFPART.m_b_default, (rdwr_body).m_ctx))
		goto L$$LBL_END;

	L$$LBL_END: (void)0;
} 


#endif
