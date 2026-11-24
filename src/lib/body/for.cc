#ifndef	aclemit_body_for_cc
#define aclemit_body_for_cc

#include <aclutil/emitx.h>
#include <aclctx/err.h>
#include <util/visitfromroot.h>

#include <ae2f/cc.h>
#include <ae2f/cgoto.h>

#include <aclval.h>
#include <aclexpr.h>
#include <aclcur/common.h>
#include <aclcur/ssa.h>
#include <aclattr/forpart.h>
#include <aclattr/branchratio.h>

#include <.aclspv/util/mkname.h>

#include <ae2f/pat/macrofunc.h>
#include <aclemit/body.h>

ae2f_MAC((L, )) aclemit_body_for(CXCursor h_cur, aclemit_body_t& rdwr_body) { do {
	aclattr_forpart_t		L$$PART;
	acltrl_t			L$$LITERAL;

	aclspv_wrd_t			L$$LOOPCTL	= SpvLoopControlMaskNone;
	aclspv_wrd_t			L$$LOOPCND	= 0;
	aclspv_wrd_t			L$$WEIGHTS[2]	= {0, 0};

	/** loop-scope frame: saved before the body, restored after it. */
	aclspv_bool_t			L$$FNRET_BODY	= 0;
	aclspv_id_t			L$$BRK_PAR	= 0,
					L$$CNT_PAR	= 0;

	L$$LITERAL.m_api_uintmax	= 0;

	(rdwr_body).m_vid = 0;

	unless(aclattr_forpart(h_cur, &L$$PART)) {
		_aclctx_err((rdwr_body).m_ctx, ACLSPV_COMPILE_FOR_LACK_COMPONENTS);
		break;
	}

	aclutil_visitfromroot(
			L$$PART.m_body
			, s_aclattr_bratio
			, &L$$WEIGHTS[0]
			);

	ae2fpat_assertassume((rdwr_body).m_ctx);

	{
		typedef aclspv_wrd_t L$$LOOPWRD_T;
		const L$$LOOPWRD_T L$$LOOPCHECK		= (rdwr_body).m_ctx->m_id++;
		const L$$LOOPWRD_T L$$LOOPSTART		= (rdwr_body).m_ctx->m_id++;
		const L$$LOOPWRD_T L$$LOOPEND		= (rdwr_body).m_ctx->m_id++;
		const L$$LOOPWRD_T L$$LOOPCONTINUE	= (rdwr_body).m_ctx->m_id++;

		/** the init runs once, in the block that reaches the loop. */
		if(L$$PART.m_has_init) {
			aclutil_visitfromroot(
					L$$PART.m_init
					, aclemit_body
					, &(rdwr_body)
					);

			ae2f_ifnezerr((rdwr_body).m_ctx->m_err)
				break;
		}

		ae2f_ifezerr(aclcur_cfg_jmp(L$$LOOPCHECK, (rdwr_body).m_ctx)) {
			_aclctx_err((rdwr_body).m_ctx, ACLSPV_COMPILE_ALLOC_FAILED);
			break;
		}

		ae2f_ifezerr(aclcur_cfg_lbl(L$$LOOPCHECK, (rdwr_body).m_ctx)) {
			_aclctx_err((rdwr_body).m_ctx, ACLSPV_COMPILE_ALLOC_FAILED);
			break;
		}

		/** same reason as the while header: the back edge is not carved
		 * when the header's loads resolve. */
		ae2f_ifezerr(aclcur_cfg_lbl_loop(L$$LOOPCHECK, (rdwr_body).m_ctx)) {
			_aclctx_err((rdwr_body).m_ctx, ACLSPV_COMPILE_ALLOC_FAILED);
			break;
		}

		/** an absent condition is an infinite loop: a constant true keeps
		 * the loop header shaped like any other. */
		if(L$$PART.m_has_cond) {
			switch(aclexpr_eval(
						&L$$LOOPCND
						, ACLSPV_ID_BOOL
						, L$$PART.m_cond
						, &L$$LITERAL
						, (rdwr_body).m_ctx
					      )) {
				default:
					ae2f_unreachable();
				case ACLEXPR_RET_FAILURE:
				case ACLEXPR_RET_NOT_THE_CASE:
					goto L$$ERR;

				case ACLEXPR_RET_SUCCESS_LITERAL:
					L$$LOOPCND = aclval_cfg_by_enum(
							L$$LITERAL.m_api_uintmax
							, (rdwr_body).m_ctx
							, ACLSPV_ID_BOOL
							);

					ae2f_ifezerr(L$$LOOPCND)
						goto L$$ERR;

					ae2f_fallthrough;

				case ACLEXPR_RET_SUCCESS_CONSTANT:
					warn_puts("Constant value on loop condition");
					ae2f_fallthrough;

				case ACLEXPR_RET_SUCCESS:
					break;
			}
		} else {
			ae2f_ifezerr(L$$LOOPCND = aclval_cfg_bool(1, (rdwr_body).m_ctx))
				goto L$$ERR;
		}

		ae2f_ifezerr(aclcur_cfg_jc(
					L$$LOOPCND
					, L$$LOOPSTART
					, L$$LOOPEND
					, L$$WEIGHTS[0]
					, L$$WEIGHTS[1]
					, L$$LOOPEND
					, L$$LOOPCONTINUE
					, L$$LOOPCTL
					, (rdwr_body).m_ctx)) {
			_aclctx_err((rdwr_body).m_ctx, ACLSPV_COMPILE_ALLOC_FAILED);
			break;
		}

		{
			ae2f_ifezerr(aclcur_cfg_lbl(L$$LOOPSTART, (rdwr_body).m_ctx))
				goto L$$ERR;

			L$$BRK_PAR	= aclcur_get_break_loop((rdwr_body).m_ctx);
			L$$CNT_PAR	= aclcur_get_continue_loop((rdwr_body).m_ctx);

			aclcur_set_fnret(0, (rdwr_body).m_ctx);
			aclcur_set_break_loop(L$$LOOPEND, (rdwr_body).m_ctx);
			/** continue reaches the increment block, not the header: the
			 * increment must run on every path that stays in the loop. */
			aclcur_set_continue_loop(L$$LOOPCONTINUE, (rdwr_body).m_ctx);

			aclutil_visitfromroot(
					L$$PART.m_body
					, aclemit_body
					, &(rdwr_body)
					);

			L$$FNRET_BODY	= aclcur_get_fnret((rdwr_body).m_ctx);

			aclcur_set_break_loop(L$$BRK_PAR, (rdwr_body).m_ctx);
			aclcur_set_continue_loop(L$$CNT_PAR, (rdwr_body).m_ctx);

			ae2f_ifnezerr((rdwr_body).m_ctx->m_err) {
				break;
			}

			unless(L$$FNRET_BODY) {
				ae2f_expected_but_else(aclcur_cfg_jmp(L$$LOOPCONTINUE, (rdwr_body).m_ctx)) {
					goto L$$ERR;
				}
			}
		}

		ae2f_ifezerr(aclcur_cfg_lbl(L$$LOOPCONTINUE, (rdwr_body).m_ctx)) {
			_aclctx_err((rdwr_body).m_ctx, ACLSPV_COMPILE_ALLOC_FAILED);
			break;
		}

		/** the increment runs in the continue block, so `continue` and the
		 * body's natural fall-through both execute it exactly once.
		 *
		 * a body that can only leave through break / return leaves the
		 * fall-through label dead (the break handler opens a fresh,
		 * unreferenced label), so nothing live reaches this block. its
		 * reads would resolve to nothing and fall back to a memory load —
		 * but a register the loop carries has no memory to load from, so
		 * the module would reference an undefined id. the increment is
		 * dead code on that path: skip it, the back edge below still
		 * shapes the loop. a `continue` inside the body keeps the block
		 * live, and then the increment must run. */
		if(L$$PART.m_has_inc
				&& aclcur_ssa_lbl_live(L$$LOOPCONTINUE, (rdwr_body).m_ctx)) {
			aclutil_visitfromroot(
					L$$PART.m_inc
					, aclemit_body
					, &(rdwr_body)
					);

			ae2f_ifnezerr((rdwr_body).m_ctx->m_err)
				break;
		}

		/** the increment is not a return: put back the frame the body
		 * left behind. LOOPEND below is opened and left unterminated, so
		 * the parent supplies the terminator (the old value made an
		 * enclosing loop skip its fall-through branch after a body that
		 * only ended in a return). */
		aclcur_set_fnret(0, (rdwr_body).m_ctx);

		ae2f_ifezerr(aclcur_cfg_jmp(L$$LOOPCHECK, (rdwr_body).m_ctx)) {
			_aclctx_err((rdwr_body).m_ctx, ACLSPV_COMPILE_ALLOC_FAILED);
			break;
		}

		ae2f_ifezerr(aclcur_cfg_lbl(L$$LOOPEND, (rdwr_body).m_ctx)) {
			_aclctx_err((rdwr_body).m_ctx, ACLSPV_COMPILE_ALLOC_FAILED);
			break;
		}

		aclutil_mkname_dbg_fn(
				L$$LOOPCHECK
				, "forcnd"
				, (rdwr_body).m_ctx
				, &(rdwr_body).m_ctx->m_tmp.m_v0
				);

		aclutil_mkname_dbg_fn(
				L$$LOOPSTART
				, "forbody"
				, (rdwr_body).m_ctx
				, &(rdwr_body).m_ctx->m_tmp.m_v0
				);

		aclutil_mkname_dbg_fn(
				L$$LOOPCONTINUE
				, "forcontinue"
				, (rdwr_body).m_ctx
				, &(rdwr_body).m_ctx->m_tmp.m_v0
				);

		aclutil_mkname_dbg_fn(
				L$$LOOPEND
				, "forbreak"
				, (rdwr_body).m_ctx
				, &(rdwr_body).m_ctx->m_tmp.m_v0
				);

		break;
	}

	ae2f_unreachable();
	L$$ERR:
		_aclctx_err((rdwr_body).m_ctx, ACLSPV_COMPILE_UNHANDLED_FOR);

	/**
	 * for(init; cond; inc) { ... }
	 *
	 * init
	 * goto LOOPCHECK;
	 *
	 * LOOPCHECK:
	 * {
	 * 	if(cond) goto LOOPSTART; else goto LOOPEND;
	 * 	LOOPSTART:
	 * 	...
	 *
	 * 	goto LOOPCONTINUE;
	 * 	LOOPCONTINUE:
	 * 	inc
	 * 	goto LOOPCHECK;
	 * }
	 *
	 * LOOPEND:
	 *
	 * */

} while(0); }

#endif
