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

#ifndef	aclattr_branchratio_h
#define aclattr_branchratio_h

#include <aclspv.h>
#include <ae2f/Keys.h>
#include <util/log.h>
#include <string.h>
#include <stdio.h>

/** 
 * @param wr_ratio : aclspv_wrd_t[2] { true_weight , false_weight }
 * */
static enum CXChildVisitResult	s_aclattr_bratio(
		CXCursor h_cur,
		ae2f_unused CXCursor h_parent,
		CXClientData ae2f_restrict wr_ratio) {
	const CXString	ATTR_KIND = clang_getCursorSpelling(h_cur);
	const char*	NIDDLE;

	dbg_puts("[s_aclattr_bratio]");
	dbg_puts(ATTR_KIND.data);

	if(h_cur.kind == CXCursor_AnnotateAttr 
			&& (NIDDLE = strstr(clang_getCString(ATTR_KIND)
					, "aclspv_bratio"))) {
		unsigned	A0 = 0, A1 = 0;

		dbg_puts("s_attr_bratio::detected");
		sscanf(NIDDLE, " aclspv_bratio ( %u , %u )"
				, &A0
				, &A1
		      );

		clang_disposeString(ATTR_KIND);

		((aclspv_wrd_t* ae2f_restrict)(wr_ratio))[0] = A0;
		((aclspv_wrd_t* ae2f_restrict)(wr_ratio))[1] = A1;
		return CXChildVisit_Break;
	}

	clang_disposeString(ATTR_KIND);

	return CXChildVisit_Recurse;
}
#endif
