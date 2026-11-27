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

#ifndef	aclattr_continue_h
#define	aclattr_continue_h

#include <aclspv.h>
#include <ae2f/c90/StdInt/enum.h>

/** @returns aclspv_bool_t */
static enum CXChildVisitResult aclattr_continue(
		CXCursor h_cur, 
		ae2f_unused CXCursor h_parent, 
		CXClientData ae2f_restrict r_does_continue_exist) {
	switch((uint_leastenum_t)h_cur.kind) {
		case CXCursor_ForStmt:
		case CXCursor_WhileStmt:
		case CXCursor_DoStmt:
		case CXCursor_CXXForRangeStmt:
			return CXChildVisit_Continue;

		default:
			return CXChildVisit_Recurse;

		case CXCursor_ContinueStmt:
			*((aclspv_bool_t* ae2f_restrict)r_does_continue_exist) = 1;
			return CXChildVisit_Break;
	}
}

#endif
