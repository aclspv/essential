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
 * @file glcmagic.h 
 * GLCompute magic (user wants auto input-thingy?)
 * */

#ifndef attr_glcmagic_h
#define attr_glcmagic_h

#include <ae2f/Keys.h>
#include <string.h>
#include <aclspv3rd/clang/idx.h>

/**
 * @brief	attr_glcmagic
 * @param	unsigned* ae2f_restrict wr_ismagic
 * 	Its value must be cleared as 0 when passed.
 * 	0 when the attribute is not found.
 * 	1 when the attribute is found.
 *
 * */
static enum CXChildVisitResult	s_attr_glcmagic(CXCursor h_cur, CXCursor h_parent, CXClientData ae2f_restrict wr_ismagic) {
	CXString TEXT;
	const char* NEEDLE;
	TEXT = clang_getCursorSpelling(h_cur);

	unless(NEEDLE = strstr(TEXT.data, "aclspv_storage_class")) {
		clang_disposeString(TEXT);
		return CXChildVisit_Recurse;
	}

	*((unsigned* ae2f_restrict)wr_ismagic) |= 1;
	clang_disposeString(TEXT);
	return CXChildVisit_Break;

	(void)h_parent;
}

#endif
