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

#include "./root.h"
#include <aclctx/err.h>
#include "./fndecl/norm.h"
#include "./fndecl/entp.h"

#include <aclspv.h>
#include <aclcur/common.h>

#include <ae2f/pat/dbg.h>
#include <aclattr/execmodel.h>
#include <aclutil/fn.h>

ae2f_inline_f static aclspv_wrd_t s_is_kernel(const CXCursor h_fndecl) {
	enum SpvExecutionModel_ MODEL = SpvExecutionModelMax;
	clang_visitChildren(h_fndecl, s_attr_execmodel, &MODEL);
	return (aclspv_wrd_t)(MODEL ^ SpvExecutionModelMax);
}

ae2f_extern enum CXChildVisitResult aclemit_root(
		CXCursor h_cur, CXCursor h_parent, CXClientData h_ctx
		) {
#define CTX	((h_aclspv_ctx_t)h_ctx)

	ae2fpat_assertassume(h_ctx);

	/** search */
	unless(h_cur.kind == CXCursor_CompoundStmt
			&& (h_parent.kind == CXCursor_FunctionDecl
				|| aclutil_fn_is_method(h_parent)))
		return CXChildVisit_Recurse;

	/** which bodies go out, and when, is the scheduler's call: see
	 * aclemit/bodies.c. reaching here means the body is being emitted now. */

	/** per-function aclcur bootstrap: the root label occupies node 0 and
	 * becomes the current label for the argument + body walk. */
	CTX->m_num_cursor = 0;

	ae2f_ifezerr(aclcur_cfg_root(ACLSPV_ID_VOID, CTX)) {
		_aclctx_err(CTX, ACLSPV_COMPILE_ALLOC_FAILED);
		return CXChildVisit_Break;
	}

	return (CTX->m_work_nxt
			= (h_parent.kind == CXCursor_FunctionDecl
				&& s_is_kernel(h_parent))
			? aclemit_fndecl_entp
			: aclemit_fndecl_norm
	       )(h_cur, h_parent, CTX->m_work_data);

#undef	CTX
}
