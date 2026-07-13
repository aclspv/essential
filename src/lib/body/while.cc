#ifndef	aclemit_body_while_cc
#define aclemit_body_while_cc

#include <util/fnctx.h>
#include <util/emitx.h>
#include <util/visitfromroot.h>

#include <ae2f/cc.h>
#include <ae2f/cgoto.h>

#include <aclexpr.h>
#include <aclcursor.h>
#include <aclattr/getkinder.h>
#include <aclattr/branchratio.h>

#include <.aclspv/util/mkname.h>

#include <ae2f/pat/macrofunc.h>
#include <aclemit/body.h>

ae2f_MAC((L, )) aclemit_body_while(CXCursor h_cur, aclutil_fnctx * ae2f_restrict h_fnctx) { do {
	CXCursor			L$$CUR[2];
	aclutil_literal			L$$LITERAL;

	struct aclattr_getkinder	L$$GETKINDER;

	aclspv_wrd_t			L$$LOOPCTL	= SpvLoopControlMaskNone;
	aclspv_wrd_t			L$$LOOPCND	= 0;
	aclspv_wrd_t			L$$SNAPSHOT	= 0;
	aclspv_wrd_t			L$$WEIGHTS[2]	= {0, 0};

	unsigned			L$$IS_INFINITE_LOOP = 0;

	L$$GETKINDER.m_cursor		= &L$$CUR[0];
	L$$GETKINDER.m_num_grepped	= 0;
	L$$GETKINDER.m_num_wanted	= 2;
	L$$GETKINDER.m_res		= CXChildVisit_Continue;

	L$$SNAPSHOT			= (h_fnctx)->m_ctx->m_count.m_fnimpl;
	L$$LITERAL.m_api_uintmax	= 0;

	clang_visitChildren(
			h_cur
			, s_aclattr_getkinder_visit
			, &L$$GETKINDER
			);

	aclutil_visitfromroot(
			L$$GETKINDER.m_cursor[1]
			, s_aclattr_bratio
			, &L$$WEIGHTS[0]
			);

	if(ae2f_expected_not_strict(((~L$$GETKINDER.m_num_grepped) >> 1) & 1)) {
		dbg_prefix();
		dbg_call((void), fprintf(stderr, "LACKCOMPONENTS: %u %u\n"
					, L$$GETKINDER.m_num_grepped
					, (((~L$$GETKINDER.m_num_grepped) >> 1) & 1)));
		(h_fnctx)->m_ctx->m_err = ACLSPV_COMPILE_WHILE_LACK_COMPONENTS;
		break;
	}

	ae2fpat_assertassume((h_fnctx));
	ae2fpat_assertassume((h_fnctx)->m_ctx);

	{
		typedef aclspv_wrd_t L$$LOOPWRD_T;
		const L$$LOOPWRD_T L$$LOOPCHECK		= (h_fnctx)->m_ctx->m_id++;
		const L$$LOOPWRD_T L$$LOOPSTART		= (h_fnctx)->m_ctx->m_id++;
		const L$$LOOPWRD_T L$$LOOPEND		= (h_fnctx)->m_ctx->m_id++;
		const L$$LOOPWRD_T L$$LOOPCONTINUE	= (h_fnctx)->m_ctx->m_id++;

		ae2f_ifnezerr((h_fnctx->m_ctx)->m_err = aclcursor_vardecl_evuclbl(
					(h_fnctx->m_ctx)->m_num_cursor
					, (h_fnctx->m_ctx)->m_cursors.m_p
					, (h_fnctx->m_ctx)
					, (h_fnctx)->m_var_stage + 1)) goto L$$RET;

		ae2f_ifezerr((h_fnctx)->m_ctx->m_count.m_fnimpl = aclutil_emitx_2(
					&(h_fnctx)->m_ctx->m_section.m_fnimpl
					, (h_fnctx)->m_ctx->m_count.m_fnimpl
					, SpvOpBranch
					,  L$$LOOPCHECK
					)) {
			(h_fnctx)->m_ctx->m_err = ACLSPV_COMPILE_ALLOC_FAILED;
			break;
		}

		ae2f_ifezerr((h_fnctx)->m_ctx->m_count.m_fnimpl = aclutil_emitx_2(
					&(h_fnctx)->m_ctx->m_section.m_fnimpl
					, (h_fnctx)->m_ctx->m_count.m_fnimpl
					, SpvOpLabel
					,  L$$LOOPCHECK
					)) {
			(h_fnctx)->m_ctx->m_err = ACLSPV_COMPILE_ALLOC_FAILED;
			break;
		}

		(h_fnctx)->m_branch = L$$LOOPCHECK;

		switch(aclexpr_eval(
					&L$$LOOPCND
					, ACLSPV_ID_BOOL
					, L$$CUR[0]
					, &L$$LITERAL
					, (h_fnctx)
				      )) {
			default:
			case ACLEXPR_RET_FAILURE:
			case ACLEXPR_RET_NOT_THE_CASE:
				goto L$$ERR;

			case ACLEXPR_RET_SUCCESS_LITERAL:
				(h_fnctx)->m_ctx->m_count.m_fnimpl
					= ae2f_bll_sel_bool1(
							aclspv_wrd_t
							, L$$LITERAL.m_api_uintmax
							, (h_fnctx)->m_ctx->m_count.m_fnimpl
							, L$$SNAPSHOT);

				ae2f_cgoto2_strict(L$$LITERAL.m_api_uintmax, L$$CONTINUE, L$$RET);

			case ACLEXPR_RET_SUCCESS_CONSTANT:
				L$$CONTINUE:
					warn_puts("Constant value on loop condition");
				L$$IS_INFINITE_LOOP = (aclspv_wrd_t)L$$LITERAL.m_api_uintmax;
				ae2f_fallthrough;

			case ACLEXPR_RET_SUCCESS:
				break;
		}

		ae2f_ifnezerr((h_fnctx->m_ctx)->m_err = aclcursor_vardecl_evuclbl(
					(h_fnctx->m_ctx)->m_num_cursor
					, (h_fnctx->m_ctx)->m_cursors.m_p
					, (h_fnctx->m_ctx)
					, (h_fnctx)->m_var_stage + 1)) goto L$$RET;


		ae2f_ifezerr((h_fnctx)->m_ctx->m_count.m_fnimpl = aclutil_emitx_4(
					&(h_fnctx)->m_ctx->m_section.m_fnimpl
					, (h_fnctx)->m_ctx->m_count.m_fnimpl
					, SpvOpLoopMerge
					,  L$$LOOPEND
					,  L$$LOOPCONTINUE
					, L$$LOOPCTL)) {
			(h_fnctx)->m_ctx->m_err = ACLSPV_COMPILE_ALLOC_FAILED;
			break;
		}

		if(L$$WEIGHTS[0] || L$$WEIGHTS[1]) {
			ae2f_ifezerr((h_fnctx)->m_ctx->m_count.m_fnimpl = aclutil_emitx_6(
						&(h_fnctx)->m_ctx->m_section.m_fnimpl
						, (h_fnctx)->m_ctx->m_count.m_fnimpl
						, SpvOpBranchConditional
						, L$$LOOPCND
						,  L$$LOOPSTART
						,  L$$LOOPEND
						, L$$WEIGHTS[0]
						, L$$WEIGHTS[1])) {
				(h_fnctx)->m_ctx->m_err = ACLSPV_COMPILE_ALLOC_FAILED;
				break;
			}
		}
		else {
			ae2f_ifezerr((h_fnctx)->m_ctx->m_count.m_fnimpl = aclutil_emitx_4(
						&(h_fnctx)->m_ctx->m_section.m_fnimpl
						, (h_fnctx)->m_ctx->m_count.m_fnimpl
						, SpvOpBranchConditional
						, L$$LOOPCND
						, L$$LOOPSTART
						, L$$LOOPEND)) {
				(h_fnctx)->m_ctx->m_err = ACLSPV_COMPILE_ALLOC_FAILED;
				break;
			}
		}

		{
			aclutil_fnctx L$$FNCTX_NEW = *h_fnctx;
			L$$FNCTX_NEW.m_state0 = 0;
			L$$FNCTX_NEW.m_break_loop	= L$$LOOPEND;
			L$$FNCTX_NEW.m_continue_loop	= L$$LOOPCHECK;

			ae2f_ifezerr((h_fnctx->m_ctx)->m_count.m_fnimpl = aclutil_emitx_2(
						&(h_fnctx->m_ctx)->m_section.m_fnimpl
						, (h_fnctx->m_ctx)->m_count.m_fnimpl
						, SpvOpLabel
						, L$$LOOPSTART
						)) goto L$$ERR;

			_aclutil_fnctx_snap(L$$FNCTX_NEW, *h_fnctx, ACLUTIL_FNCTX_SNAPS_LBL);

			(h_fnctx)->m_branch = L$$LOOPSTART;
			(void)++L$$FNCTX_NEW.m_var_stage;

			aclutil_visitfromroot(
					L$$CUR[1]
					, aclemit_entp_body
					, &L$$FNCTX_NEW
					);

			ae2f_ifnezerr(((h_fnctx)->m_ctx)->m_err) {
				break;
			}

			h_fnctx->m_state0		|= L$$FNCTX_NEW.m_state0;

			ae2f_ifnezerr((h_fnctx->m_ctx)->m_err = aclcursor_vardecl_evuclbl(
						(h_fnctx->m_ctx)->m_num_cursor
						, (h_fnctx->m_ctx)->m_cursors.m_p
						, (h_fnctx->m_ctx)
						, (h_fnctx)->m_var_stage + 1)) goto L$$RET;

			unless(L$$FNCTX_NEW.m_state0 & ACLUTIL_FNCTX_STATE0_FNRET) {
				ae2f_expected_but_else((h_fnctx->m_ctx)->m_count.m_fnimpl = aclutil_emitx_2(
							&(h_fnctx->m_ctx)->m_section.m_fnimpl
							, (h_fnctx->m_ctx)->m_count.m_fnimpl
							, SpvOpBranch
							, L$$LOOPCONTINUE
							)) {
					goto L$$ERR;
				}
			}

			ae2f_ifnezerr(L$$IS_INFINITE_LOOP & L$$FNCTX_NEW.m_state0 >> (aclspv_wrd_t)ACLUTIL_FNCTX_STATEPAD0_BREAK_USED) {
				(h_fnctx)->m_ctx->m_err = ACLSPV_COMPILE_INFINITE_LOOP_FOUND;
				break;
			}
		}

		ae2f_ifezerr((h_fnctx)->m_ctx->m_count.m_fnimpl = aclutil_emitx_2(
					&(h_fnctx)->m_ctx->m_section.m_fnimpl
					, (h_fnctx)->m_ctx->m_count.m_fnimpl
					, SpvOpLabel
					, L$$LOOPCONTINUE
					)) {
			(h_fnctx)->m_ctx->m_err = ACLSPV_COMPILE_ALLOC_FAILED;
			break;
		}
		(h_fnctx)->m_branch = L$$LOOPCONTINUE;

		ae2f_ifezerr((h_fnctx)->m_ctx->m_count.m_fnimpl = aclutil_emitx_2(
					&(h_fnctx)->m_ctx->m_section.m_fnimpl
					, (h_fnctx)->m_ctx->m_count.m_fnimpl
					, SpvOpBranch
					, L$$LOOPCHECK
					)) {
			(h_fnctx)->m_ctx->m_err = ACLSPV_COMPILE_ALLOC_FAILED;
			break;
		}

		ae2f_ifezerr((h_fnctx)->m_ctx->m_count.m_fnimpl = aclutil_emitx_2(
					&(h_fnctx)->m_ctx->m_section.m_fnimpl
					, (h_fnctx)->m_ctx->m_count.m_fnimpl
					, SpvOpLabel
					, L$$LOOPEND
					)) {
			(h_fnctx)->m_ctx->m_err = ACLSPV_COMPILE_ALLOC_FAILED;
			break;
		}

		(h_fnctx)->m_branch = L$$LOOPEND;

		aclutil_mkname_dbg_fn(
				L$$LOOPCHECK
				, "whilecnd"
				, (h_fnctx)->m_ctx
				, &(h_fnctx)->m_ctx->m_tmp.m_v0
				);

		aclutil_mkname_dbg_fn(
				L$$LOOPSTART
				, "whilebody"
				, (h_fnctx)->m_ctx
				, &(h_fnctx)->m_ctx->m_tmp.m_v0
				);

		aclutil_mkname_dbg_fn(
				L$$LOOPCONTINUE
				, "whilecontinue"
				, (h_fnctx)->m_ctx
				, &(h_fnctx)->m_ctx->m_tmp.m_v0
				);

		aclutil_mkname_dbg_fn(
				L$$LOOPEND
				, "whilebreak"
				, (h_fnctx)->m_ctx
				, &(h_fnctx)->m_ctx->m_tmp.m_v0
				);

		break;
	}

	ae2f_unreachable();
	L$$ERR:
		(h_fnctx)->m_ctx->m_err = (e_aclspv_compile_t) ae2f_bll_sel_bool_naive1(
				uint_leastenum_t
				, (h_fnctx)->m_ctx->m_err
				, (uint_leastenum_t) (h_fnctx)->m_ctx->m_err
				, ACLSPV_COMPILE_UNHANDLED_WHILE
				);

	/**
	 * while(c) { ... }
	 * 
	 * LOOPCHECK:
	 * {
	 * 	if(c) goto LOOPSTART; else goto LOOPEND;
	 * 	LOOPSTART:
	 * 	...
	 *
	 * 	goto LOOPCHECK;
	 * }
	 *
	 * LOOPEND:
	 *
	 * */

	L$$RET: (void)0;
} while(0); }

#endif
