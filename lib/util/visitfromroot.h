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

#ifndef	aclutil_visitfromroot_h
#define aclutil_visitfromroot_h

#include <ae2f/cc.h>
#include <ae2f/cgoto.h>

#include <aclspv.h>

ae2f_always_inline static enum CXChildVisitResult aclutil_visitfromroot(
		CXCursor	c_cursor,
		CXCursorVisitor	fn_visit,
		CXClientData	h_clientdata
		) {
	enum CXChildVisitResult R = fn_visit(c_cursor, c_cursor, h_clientdata);

	if(R == CXChildVisit_Recurse) {
		R = (enum CXChildVisitResult) clang_visitChildren(c_cursor, fn_visit, h_clientdata);
	}

	return R;
}

#endif
