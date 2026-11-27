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

#include "./fn.h"
#include <aclctx/err.h>
#include "./fn/arg.h"
#include "./fn/build.h"

#include "./implutil.h"
#include "./by_cursor.h"
#include "./default.h"
#include "./ptr.h"

#include <aclutil/fn.h>


#if	0
static void s_cur_dcr(aclutil_cxtp* cxtp, CXCursor c_cur, CXType c_ty, h_aclspv_ctx_t h_ctx) {

}
#endif

aclspv_id_t aclty_mk_fn(
		const aclspv_id_t	c_ret_id,
		const h_aclspv_ctx_t	h_ctx
		)
{
	aclty_t* cxtp = aclty_cfg_fn(c_ret_id, h_ctx);
	ae2f_ifezerr(cxtp)	return 0;
	return aclty_build_fn(cxtp, h_ctx);
}

aclspv_id_t aclty_mk_fn1(
		const CXCursor		c_cursor_compound,
		const CXType		c_type_compound,
		const h_aclspv_ctx_t	h_ctx
		)
{
	int ARGC			= clang_getNumArgTypes(c_type_compound);
	aclspv_wrd_t	ARGC_IDX;
	aclspv_id_t	RETID;
	aclty_t* fn;
	aclspv_wrd_t	pad;

	{
		const CXType RET	= clang_getResultType(c_type_compound);

		/** void is not a type entry: by-cursor resolves scalars through
		 * the type arena, and there is nothing to resolve for a function
		 * that returns nothing. same construction as the entry-point
		 * emitter. */
		RETID = (RET.kind == CXType_Void)
			? aclty_mk_default(ACLSPV_ID_VOID, h_ctx)
			: aclty_mk_by_cursor_default(
					c_cursor_compound
					, RET
					, h_ctx
					);

		ae2f_expected_but_else(RETID) return 0;

	}


	ae2f_ifnezerr_strict(ARGC <= 0) {
		return aclty_mk_fn(RETID, h_ctx);
	}	/** fallback: argc was under 0 */

	ae2f_ifezerr(fn = aclty_cfg_fn(RETID, h_ctx))
		return 0;


	pad = (aclspv_wrd_t)(fn - __get_type_uniques(h_ctx));

	/** source order. the parameter's own cursor is passed to the type
	 * builder so a per-parameter annotation (aclspv_storage_class, ...) is
	 * read from the declaration that carries it, not from the function. */
	for(ARGC_IDX = 0; ARGC_IDX < (aclspv_wrd_t)ARGC; ++ARGC_IDX) {
		const CXCursor ARG_CUR	= clang_Cursor_getArgument(
				c_cursor_compound
				, (unsigned)ARGC_IDX
				);
		const CXType ARG		= clang_getArgType(
				c_type_compound
				, ARGC_IDX
				);

		const aclspv_wrd_t ARG_ID	= aclty_mk_by_cursor_default(
				clang_Cursor_isNull(ARG_CUR)
					? c_cursor_compound
					: ARG_CUR
				, ARG
				, h_ctx
				);

		ae2f_expected_but_else(ARG_ID)	{
			_aclctx_err(h_ctx, ACLSPV_COMPILE_CXTP_ZERO);
			return 0;
		}

		/** creating the argument's type may have grown the arena:
		 * re-resolve the function node before touching it. */
		fn = &__get_type_uniques(h_ctx)[pad];

		ae2f_ifezerr(aclty_add_fn_arg(ARG_ID, h_ctx, fn))
			return 0;
	}

	/** adding an argument may have grown the arena, too. */
	fn = &__get_type_uniques(h_ctx)[pad];

	return aclty_build_fn(fn, h_ctx);
}

aclspv_id_t aclty_mk_fn1_this(
		const CXCursor		c_cursor_compound,
		const CXType		c_type_compound,
		const h_aclspv_ctx_t	h_ctx
		)
{
	const CXCursor PARENT = clang_getCursorSemanticParent(c_cursor_compound);
	const CXType PARENT_TY = clang_getCursorType(PARENT);
	const int ARGC = clang_getNumArgTypes(c_type_compound);
	aclspv_id_t RETID, THIS_TY, THIS_PTR, ARG_ID;
	aclspv_wrd_t	ARGC_IDX, pad;
	aclty_t* fn;

	unless(aclutil_fn_has_this(c_cursor_compound))
		return aclty_mk_fn1(c_cursor_compound, c_type_compound, h_ctx);

	{
		const CXType RET = clang_getResultType(c_type_compound);

		RETID = (RET.kind == CXType_Void)
			? aclty_mk_default(ACLSPV_ID_VOID, h_ctx)
			: aclty_mk_by_cursor_default(c_cursor_compound, RET, h_ctx);

		ae2f_expected_but_else(RETID) return 0;
	}

	ae2f_ifezerr(THIS_TY = aclty_mk_by_cursor_default(PARENT, PARENT_TY, h_ctx))
		return 0;

	ae2f_ifezerr(THIS_PTR = aclty_mk_ptr(THIS_TY, SpvStorageClassFunction, h_ctx))
		return 0;

	ae2f_ifezerr(fn = aclty_cfg_fn(RETID, h_ctx))
		return 0;

	pad = (aclspv_wrd_t)(fn - __get_type_uniques(h_ctx));

	ae2f_ifezerr(aclty_add_fn_arg(THIS_PTR, h_ctx, fn))
		return 0;

	for(ARGC_IDX = 0; ARGC_IDX < (aclspv_wrd_t)ARGC; ++ARGC_IDX) {
		const CXCursor ARG_CUR = clang_Cursor_getArgument(
				c_cursor_compound
				, (unsigned)ARGC_IDX
				);
		const CXType ARG = clang_getArgType(c_type_compound, ARGC_IDX);

		ARG_ID = aclty_mk_by_cursor_default(
				clang_Cursor_isNull(ARG_CUR) ? c_cursor_compound : ARG_CUR
				, ARG
				, h_ctx
				);

		ae2f_expected_but_else(ARG_ID) {
			_aclctx_err(h_ctx, ACLSPV_COMPILE_CXTP_ZERO);
			return 0;
		}

		fn = &__get_type_uniques(h_ctx)[pad];

		ae2f_ifezerr(aclty_add_fn_arg(ARG_ID, h_ctx, fn))
			return 0;
	}

	fn = &__get_type_uniques(h_ctx)[pad];

	return aclty_build_fn(fn, h_ctx);
}
