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

/** @file export.h
 *
 * whether a function is meant to leave the module: `__declspec(dllexport)`
 * (a CXCursor_DLLExport child) or `__attribute__((visibility("default")))`
 * (a CXCursor_VisibilityAttr whose value is `default`).
 *
 * `visibility("hidden")` is the explicit opposite: it overrides an
 * export-by-default policy.
 * */

#ifndef	aclattr_export_h
#define	aclattr_export_h

#include <aclspv.h>
#include <ae2f/cc.h>
#include <ae2f/c90/stdint.h>

typedef struct {
	aclspv_bool_t	m_export;
	aclspv_bool_t	m_hidden;
} aclattr_export_t;

/** case-insensitive ASCII compare, for the attribute's value spelling. */
static aclspv_bool_t aclattr_streq_ci(const char* rd_a, const char* rd_b) {
	ae2f_ifezerr(rd_a)	return 0;
	ae2f_ifezerr(rd_b)	return 0;

	while(*rd_a && *rd_b) {
		char a = *rd_a++;
		char b = *rd_b++;

		if(a >= 'A' && a <= 'Z') a = (char)(a - 'A' + 'a');
		if(b >= 'A' && b <= 'Z') b = (char)(b - 'A' + 'a');

		if(a != b)
			return 0;
	}

	return (aclspv_bool_t)(*rd_a == *rd_b);
}

static enum CXChildVisitResult aclattr_export(
		CXCursor		c_cur,
		ae2f_unused CXCursor	c_par,
		CXClientData		rdwr_export
		) {
	aclattr_export_t* ae2f_restrict p = rdwr_export;

	switch((uintmax_t)c_cur.kind) {
		default:
			return CXChildVisit_Recurse;

		/** `__declspec(dllexport)`. */
		case CXCursor_DLLExport:
			p->m_export = 1;
			return CXChildVisit_Continue;

		case CXCursor_VisibilityAttr:
			{
				CXString	SPELL = clang_getCursorSpelling(c_cur);
				const char*	c_val = clang_getCString(SPELL);

				if(aclattr_streq_ci(c_val, "default"))
					p->m_export = 1;
				else if(aclattr_streq_ci(c_val, "hidden"))
					p->m_hidden = 1;

				clang_disposeString(SPELL);
				return CXChildVisit_Continue;
			}
	}
}

/** the declared export intent of c_fn; both zero when it carries neither. */
static aclattr_export_t aclattr_get_export(CXCursor c_fn) {
	aclattr_export_t r;

	r.m_export	= 0;
	r.m_hidden	= 0;

	clang_visitChildren(c_fn, aclattr_export, &r);

	return r;
}

#endif
