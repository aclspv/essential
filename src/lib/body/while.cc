/*
 * Copyright 2026 dalmurii
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * See the LICENSE file for the full text, including LLVM Exceptions.
 */

#ifndef	aclemit_body_while_cc
#define aclemit_body_while_cc

#include <aclutil/emitx.h>
#include <aclctx/err.h>
#include <util/visitfromroot.h>

#include <ae2f/cc.h>
#include <ae2f/cgoto.h>

#include <aclval.h>
#include <aclexpr.h>
#include <aclcur/common.h>
#include <aclattr/getkinder.h>
#include <aclattr/branchratio.h>

#include <.aclspv/util/mkname.h>

#include <ae2f/pat/macrofunc.h>
#include <aclemit/body.h>

ae2f_MAC((L, )) aclemit_body_while(CXCursor h_cur, aclemit_body_t& rdwr_body) { do {
	CXCursor			L$$CUR[2];
	acltrl_t			L$$LITERAL;

	struct aclattr_getkinder	L$$GETKINDER;

	aclspv_wrd_t			L$$LOOPCTL	= SpvLoopControlMaskNone;
	aclspv_wrd_t			L$$LOOPCND	= 0;
	aclspv_wrd_t			L$$WEIGHTS[2]	= {0, 0};

	/** loop-scope frame: saved before the body, restored after it. */
	aclspv_bool_t			L$$FNRET_BODY	= 0;
	aclspv_id_t			L$$BRK_PAR	= 0,
					L$$CNT_PAR	= 0;

	L$$GETKINDER.m_cursor		= &L$$CUR[0];
	L$$GETKINDER.m_num_grepped	= 0;
	L$$GETKINDER.m_num_wanted	= 2;
	L$$GETKINDER.m_res		= CXChildVisit_Continue;

	L$$LITERAL.m_api_uintmax	= 0;

	(rdwr_body).m_vid = 0;

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
		_aclctx_err((rdwr_body).m_ctx, ACLSPV_COMPILE_WHILE_LACK_COMPONENTS);
		break;
	}

	ae2fpat_assertassume((rdwr_body).m_ctx);

	{
		typedef aclspv_wrd_t L$$LOOPWRD_T;
		const L$$LOOPWRD_T L$$LOOPCHECK		= (rdwr_body).m_ctx->m_id++;
		const L$$LOOPWRD_T L$$LOOPSTART		= (rdwr_body).m_ctx->m_id++;
		const L$$LOOPWRD_T L$$LOOPEND		= (rdwr_body).m_ctx->m_id++;
		const L$$LOOPWRD_T L$$LOOPCONTINUE	= (rdwr_body).m_ctx->m_id++;

		

		ae2f_ifezerr(aclcur_cfg_jmp(L$$LOOPCHECK, (rdwr_body).m_ctx)) {
			_aclctx_err((rdwr_body).m_ctx, ACLSPV_COMPILE_ALLOC_FAILED);
			break;
		}

		ae2f_ifezerr(aclcur_cfg_lbl(L$$LOOPCHECK, (rdwr_body).m_ctx)) {
			_aclctx_err((rdwr_body).m_ctx, ACLSPV_COMPILE_ALLOC_FAILED);
			break;
		}

		/** the loop header (cond) is marked at carve time because the
		 * back edge is not carved yet when its loads resolve — the graph
		 * cannot reveal the loopness then. the body needs no marker: its
		 * loopness is detected from the loop JC (carved before the body
		 * is visited), whose continue target proves a cycle. */
		ae2f_ifezerr(aclcur_cfg_lbl_loop(L$$LOOPCHECK, (rdwr_body).m_ctx)) {
			_aclctx_err((rdwr_body).m_ctx, ACLSPV_COMPILE_ALLOC_FAILED);
			break;
		}

		switch(aclexpr_eval(
					&L$$LOOPCND
					, ACLSPV_ID_BOOL
					, L$$CUR[0]
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
			/** continue must reach the loop's continue block, not the
			 * header: a direct header jump adds a second back edge and
			 * violates structured control flow (one back edge per loop). */
			aclcur_set_continue_loop(L$$LOOPCONTINUE, (rdwr_body).m_ctx);

			aclutil_visitfromroot(
					L$$CUR[1]
					, aclemit_body
					, &(rdwr_body)
					);

			L$$FNRET_BODY	= aclcur_get_fnret((rdwr_body).m_ctx);

			aclcur_set_break_loop(L$$BRK_PAR, (rdwr_body).m_ctx);
			aclcur_set_continue_loop(L$$CNT_PAR, (rdwr_body).m_ctx);
			/** LOOPEND below is opened and left unterminated: the parent
			 * supplies the terminator. reporting the body's fnret here
			 * made an enclosing loop skip its fall-through branch when the
			 * body only ended in a return, leaving LOOPEND unterminated. */
			aclcur_set_fnret(0, (rdwr_body).m_ctx);

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
				, "whilecnd"
				, (rdwr_body).m_ctx
				, &(rdwr_body).m_ctx->m_tmp.m_v0
				);

		aclutil_mkname_dbg_fn(
				L$$LOOPSTART
				, "whilebody"
				, (rdwr_body).m_ctx
				, &(rdwr_body).m_ctx->m_tmp.m_v0
				);

		aclutil_mkname_dbg_fn(
				L$$LOOPCONTINUE
				, "whilecontinue"
				, (rdwr_body).m_ctx
				, &(rdwr_body).m_ctx->m_tmp.m_v0
				);

		aclutil_mkname_dbg_fn(
				L$$LOOPEND
				, "whilebreak"
				, (rdwr_body).m_ctx
				, &(rdwr_body).m_ctx->m_tmp.m_v0
				);

		break;
	}

	ae2f_unreachable();
	L$$ERR:
		_aclctx_err((rdwr_body).m_ctx, ACLSPV_COMPILE_UNHANDLED_WHILE);

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

} while(0); }

#endif
