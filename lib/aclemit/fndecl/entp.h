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

#ifndef	aclemit_fndecl_entp_h
#define aclemit_fndecl_entp_h


#include <aclspv.h>
enum CXChildVisitResult aclemit_fndecl_entp(CXCursor h_cur, CXCursor h_parent, CXClientData h_ctx);

/**
 * true when the entry point `c_entp` declares is already in this module: an
 * OpEntryPoint and a body were emitted for a function of the same name.
 *
 * an entry point is not a call target and carries no function symbol, so this
 * list is the only record of it. a context outlives one translation unit and
 * the build phase re-walks the translation unit the compile phase saw, so
 * without this test the second walk would push a second OpEntryPoint for one
 * function.
 * */
aclspv_bool_t aclemit_fndecl_entp_exists(
		CXCursor		c_entp,
		h_aclspv_ctx_t		h_ctx
		);

/** the same question from the cursor mark alone, for a caller that no longer
 * holds a cursor. */
aclspv_bool_t aclemit_fndecl_entp_exists_by_mark(
		aclspv_wrd_t		c_mark,
		h_aclspv_ctx_t		h_ctx
		);

#endif
