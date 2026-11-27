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

/** @file specid.h */

#ifndef lib_attr_specid_h
#define lib_attr_specid_h

#include <ae2f/Keys.h>
#include <stdio.h>
#include <string.h>
#include <aclspv.h>

static enum CXChildVisitResult	s_attr_specid(CXCursor h_cur, ae2f_unused CXCursor h_parent, CXClientData ae2f_restrict wr_specid) {
	CXString TEXT;
	const char* NEEDLE;
	unsigned SPECID = *((aclspv_wrd_t* ae2f_restrict)wr_specid);

	if(SPECID != 0xFFFFFFFF)
		return CXChildVisit_Break;

	unless(h_cur.kind == CXCursor_AnnotateAttr) return CXChildVisit_Recurse;
	TEXT = clang_getCursorSpelling(h_cur);

	unless(NEEDLE = strstr(TEXT.data, "aclspv_specid")) {
		clang_disposeString(TEXT);
		return CXChildVisit_Recurse;
	}

	SPECID = 0;
	sscanf(NEEDLE, "aclspv_specid ( %u )", &SPECID);

	*((aclspv_wrd_t* ae2f_restrict)wr_specid) = (aclspv_wrd_t)SPECID;

	clang_disposeString(TEXT);
	return CXChildVisit_Break;
}

#endif
