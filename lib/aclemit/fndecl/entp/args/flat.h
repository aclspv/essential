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

#ifndef	aclemit_fndecl_entp_args_flat_h
#define aclemit_fndecl_entp_args_flat_h

#include <ae2f/Keys.h>
#include <string.h>
#include <aclspv.h>

/**
 * detect a bare `aclspv_flat` annotation on the cursor.
 *
 * writes 1 to *wr_flat when found and stops the visit; leaves the
 * destination untouched otherwise.
 *
 * @fn static enum CXChildVisitResult s_attr_flat(
 *     CXCursor h_cur,
 *     CXCursor h_parent,
 *     aclspv_wrd_t* ae2f_restrict wr_flat
 * )
 * */
static enum CXChildVisitResult	s_attr_flat(
		CXCursor h_cur,
		ae2f_unused CXCursor h_parent,
		CXClientData ae2f_restrict wr_flat) {
	CXString TEXT;

	unless(h_cur.kind == CXCursor_AnnotateAttr)
		return CXChildVisit_Recurse;

	TEXT = clang_getCursorSpelling(h_cur);

	unless(strstr(TEXT.data, "aclspv_flat")) {
#if __ACLSPV_ATTR_ALIASES
		/** `annotate("flat")` */
		if(!strcmp(TEXT.data, "flat")) {
			*((aclspv_wrd_t* ae2f_restrict)wr_flat) = 1;
			clang_disposeString(TEXT);
			return CXChildVisit_Break;
		}
#endif
		clang_disposeString(TEXT);
		return CXChildVisit_Continue;
	}

	*((aclspv_wrd_t* ae2f_restrict)wr_flat) = 1;

	clang_disposeString(TEXT);
	return CXChildVisit_Break;
}

#endif
