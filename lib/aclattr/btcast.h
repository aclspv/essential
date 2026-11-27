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

/** @file btcast.h */

#ifndef	attr_btcast_h
#define attr_btcast_h

#include <ae2f/cc.h>
#include <string.h>
#include <util/log.h>

#include <aclspv.h>

/** 
 * @fn static enum CXChildVisitResult attr_execmodel(CXCursor h_cur, CXCursor h_parent, aclspv_bool_t* ae2f_restrict wr_is_btcast)
 * */
static enum CXChildVisitResult aclattr_btcast(
		CXCursor h_cur, 
		ae2f_unused CXCursor h_parent, 
		CXClientData ae2f_restrict wr_is_btcast) {
	CXString	ATTR_KIND;
	const char*	NIDDLE;


	ATTR_KIND = clang_getCursorSpelling(h_cur);

	dbg_prefix(a);
	dbg_call(printf, ("\t%s\n", clang_getCString(ATTR_KIND)));

	if(h_cur.kind == CXCursor_AnnotateAttr && (NIDDLE = strstr(clang_getCString(ATTR_KIND), "aclspv_btcast"))) {
		dbg_puts("s_attr_btcast::detected");
		*((aclspv_bool_t* ae2f_restrict)wr_is_btcast) = 1;

		clang_disposeString(ATTR_KIND);
		return CXChildVisit_Break;
	}

	clang_disposeString(ATTR_KIND);


	return CXChildVisit_Recurse;
}

#endif
