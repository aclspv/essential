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

#ifndef aclattr_rdattr_cc
#define aclattr_rdattr_cc

#include <ae2f/pat/macrofunc.h>
#include <string.h>
#include <aclspv.h>

ae2f_MAC((L, L_NDL, )) aclattr_rdattr(
		const CXCursor c_cur,
		const char* const ae2f_restrict rd_ndl,
		enum CXChildVisitResult&	ret,
		const enum CXChildVisitResult	c_on_found,
		const enum CXChildVisitResult	c_on_not_found,
		int __scanf
		) {
	CXString			L$$ATTRSPL	= clang_getCursorSpelling(c_cur);
	const char* ae2f_restrict	L_NDL		= clang_getCString(L$$ATTRSPL) ? 
		strstr(
				clang_getCString(L$$ATTRSPL)
				, rd_ndl) : 0;

	if(L_NDL) {
		(void)__scanf;
		ret	= c_on_found;
	} else {
		ret	= c_on_not_found;
	}

	clang_disposeString(L$$ATTRSPL);
}

#endif
