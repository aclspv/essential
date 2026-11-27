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

/** 
 * @file	location.h 
 * @brief	location attribute detector
 * */

#ifndef lib_attr_location_h
#define lib_attr_location_h

#include <aclspv.h>
#include <stdio.h>
#include <string.h>

/** 
 * @fn static enum CXChildVisitResult	attr_location(CXCursor h_cur, CXCursor h_parent, aclspv_wrd_t* ae2f_restrict wr_location)
 * */
static enum CXChildVisitResult	s_attr_location(CXCursor h_cur, CXCursor h_parent, CXClientData ae2f_restrict wr_location) {
	CXString TEXT;
	enum CXChildVisitResult RES = CXChildVisit_Break;
	const char* NEEDLE;
	unsigned LOCATION;

	if(*((aclspv_wrd_t* ae2f_restrict)wr_location) != 0xFFFFFFFF)
		return CXChildVisit_Break;

	unless(h_cur.kind == CXCursor_AnnotateAttr) return CXChildVisit_Recurse;
	TEXT = clang_getCursorSpelling(h_cur);

	unless(NEEDLE = strstr(TEXT.data, "aclspv_location")) {
		RES = CXChildVisit_Continue;
		goto LBL_FINI;
	}


	sscanf(NEEDLE, "aclspv_location ( %u )", &LOCATION);

	*((aclspv_wrd_t* ae2f_restrict)wr_location) = (aclspv_wrd_t)LOCATION;

LBL_FINI:
	clang_disposeString(TEXT);
	return RES;

	(void)h_parent;
}

#endif
