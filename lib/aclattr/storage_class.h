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

/** @file storage_class.h */

#ifndef lib_attr_storage_class_h
#define lib_attr_storage_class_h

#include <ae2f/Keys.h>
#include <stdio.h>
#include <string.h>
#include <aclspv.h>

/** 
 * @fn	
 * static enum CXChildVisitResult	attr_storage_class(
 * CXCursor h_cur, 
 * CXCursor h_parent, 
 * aclspv_wrd_t* ae2f_restrict wr_storage_class
 * )
 * */
static enum CXChildVisitResult	s_attr_storage_class(CXCursor h_cur, ae2f_unused CXCursor h_un, CXClientData ae2f_restrict wr_storage_class) {
	CXString TEXT;
	enum CXChildVisitResult RES = CXChildVisit_Break;
	const char* NEEDLE;
	unsigned INP = 0;

	unless(h_cur.kind == CXCursor_AnnotateAttr)
		return CXChildVisit_Recurse;

	TEXT = clang_getCursorSpelling(h_cur);

	unless(NEEDLE = strstr(TEXT.data, "aclspv_storage_class")) {
		RES = CXChildVisit_Continue;
		goto LBL_FINI;
	}

	sscanf(NEEDLE, "aclspv_storage_class ( %u )", &INP);
	*((aclspv_wrd_t* ae2f_restrict)wr_storage_class) = (aclspv_wrd_t)INP;

LBL_FINI:
	clang_disposeString(TEXT);
	return RES;

}

#endif
