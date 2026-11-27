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

#ifndef	aclemit_body_if_cc
#define aclemit_body_if_cc

#include <aclattr/ifpart.h>
#include <aclctx/err.h>
#include <aclattr/branchratio.h>

#include <aclexpr.h>
#include <aclcur/common.h>

#include <aclemit/body.h>
#include <aclemit/inline.h>
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

	/** fnret of each arm: did it fail to fall through to the merge? */
	aclspv_bool_t		L$$FNRET_TRUE = 0,
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
		_aclctx_err((rdwr_body).m_ctx, ACLSPV_COMPILE_IF_COND_INVAL);
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

	unless(aclemit_inline_merge_push((rdwr_body).m_ctx, L$$IFPART.m_b_default))
		goto L$$LBL_END;

	{
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

		/** a slice with no fall-through (an inlined body that returned
		 * inside this arm) already ended the block: adding the merge edge
		 * beside the exit branch would leave a terminator twice. */
		if(!L$$FNRET_TRUE && !aclcur_lbl_is_fnterm((rdwr_body).m_ctx)) {
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

		if(!L$$FNRET_FALSE && !aclcur_lbl_is_fnterm((rdwr_body).m_ctx)) {
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

	aclemit_inline_merge_pop((rdwr_body).m_ctx);

	/** the merge label below is opened and left unterminated: fall-through
	 * here means "the current block still needs its terminator", which is
	 * what the parent (or the function tail) supplies. it used to be 1 as
	 * soon as one arm returned, so an enclosing loop skipped its own
	 * fall-through branch and left the live merge block without a
	 * terminator. */
	aclcur_set_fnret(0, (rdwr_body).m_ctx);

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
