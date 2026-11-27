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

#ifndef	aclattr_main_h
#define aclattr_main_h

#include <aclspv.h>
#include <ae2f/pat/dbg.h>

ae2f_unused static enum CXChildVisitResult s_aclattr_main(
		CXCursor h_cur, CXCursor h_parent, CXClientData h_ctx
		) {
	h_aclspv_ctx_t	ctx = h_ctx;
	ae2fpat_assertassume(ctx);

	return ctx->m_work_nxt ? ctx->m_work_nxt(
			h_cur
			, h_parent
			, ctx->m_work_data
			) : CXChildVisit_Break;
}

#endif
