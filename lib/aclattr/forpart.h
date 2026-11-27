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

#ifndef	aclattr_forpart_h
#define	aclattr_forpart_h

#include <aclspv.h>

/** the parts of a `for` statement.
 *
 * libclang yields the immediate children of a ForStmt in source order and
 * simply omits an absent part, so a child index alone cannot tell an init
 * expression from a condition or an increment: `for(;;i++)` and `for(;i<n;)`
 * both yield one expression child before the body. the two top-level
 * semicolons of the header are located by tokenizing the statement, and each
 * child's start offset then places it in the init, condition or increment
 * slot. */
struct s_aclattr_forpart_t {
	CXCursor	m_init;
	CXCursor	m_cond;
	CXCursor	m_inc;
	CXCursor	m_body;

	aclspv_bool_t	m_has_init;
	aclspv_bool_t	m_has_cond;
	aclspv_bool_t	m_has_inc;
};

typedef struct s_aclattr_forpart_t aclattr_forpart_t;

struct s_aclattr_forpart_children_t {
	CXCursor	m_p[4];
	aclspv_wrd_t	m_n;
};

static enum CXChildVisitResult s_aclattr_forpart_child(
		CXCursor		c_cur,
		ae2f_unused CXCursor	c_par,
		CXClientData		rdwr_data
		) {
	struct s_aclattr_forpart_children_t* ae2f_restrict p = rdwr_data;

	if(p->m_n < 4)
		p->m_p[p->m_n] = c_cur;

	++p->m_n;

	return CXChildVisit_Continue;
}

/** the file offsets of the two top-level `;` of a for header.
 *
 * depth starts at 0; the header's own `(` brings it to 1, so semicolons of
 * nested expressions (a call inside the init, a statement expression) are
 * ignored. the scan stops as soon as both separators are found, well before
 * the body. */
static aclspv_bool_t s_aclattr_forpart_seps(
		CXCursor	c_for,
		aclspv_wrd_t*	wr_semi1,
		aclspv_wrd_t*	wr_semi2
		) {
	CXTranslationUnit	TU = clang_Cursor_getTranslationUnit(c_for);
	CXSourceRange		R;
	CXToken*		tok = 0;
	unsigned		n = 0;
	unsigned		i;
	unsigned		depth = 0;
	aclspv_wrd_t		got = 0;

	if(!TU)
		return 0;

	R = clang_getCursorExtent(c_for);
	clang_tokenize(TU, R, &tok, &n);

	if(!tok)
		return 0;

	for(i = 0; i < n && got < 2; ++i) {
		CXString	sp = clang_getTokenSpelling(TU, tok[i]);
		const char*	s = clang_getCString(sp);

		if(s && s[0] == '(' && s[1] == '\0') {
			++depth;
		} else if(s && s[0] == ')' && s[1] == '\0') {
			if(depth) --depth;
		} else if(s && s[0] == ';' && s[1] == '\0' && depth == 1) {
			CXSourceRange	tr = clang_getTokenExtent(TU, tok[i]);
			unsigned	off = 0;

			clang_getFileLocation(clang_getRangeStart(tr), 0, 0, 0, &off);

			if(got == 0)	*wr_semi1 = off;
			else		*wr_semi2 = off;

			++got;
		}

		clang_disposeString(sp);
	}

	clang_disposeTokens(TU, tok, n);

	return (aclspv_bool_t)(got == 2);
}

/** split a ForStmt into its init / condition / increment / body.
 *
 * returns 0 when the shape cannot be resolved (no separators, a duplicated
 * slot, more children than the grammar allows). */
static aclspv_bool_t aclattr_forpart(
		CXCursor		c_for,
		aclattr_forpart_t*	rdwr_p
		) {
	struct s_aclattr_forpart_children_t	CH;
	aclspv_wrd_t				S1 = 0, S2 = 0;
	aclspv_wrd_t				i;

	rdwr_p->m_has_init = 0;
	rdwr_p->m_has_cond = 0;
	rdwr_p->m_has_inc = 0;

	CH.m_n = 0;
	clang_visitChildren(c_for, s_aclattr_forpart_child, &CH);

	/** at most init + cond + inc + body, and the body is always present. */
	if(CH.m_n < 1 || CH.m_n > 4)
		return 0;

	rdwr_p->m_body = CH.m_p[CH.m_n - 1];

	if(!s_aclattr_forpart_seps(c_for, &S1, &S2))
		return 0;

	for(i = 0; i + 1 < CH.m_n; ++i) {
		CXSourceRange	r = clang_getCursorExtent(CH.m_p[i]);
		unsigned	off = 0;

		clang_getFileLocation(clang_getRangeStart(r), 0, 0, 0, &off);

		if(off < S1) {
			if(rdwr_p->m_has_init) return 0;
			rdwr_p->m_init = CH.m_p[i];
			rdwr_p->m_has_init = 1;
		} else if(off < S2) {
			if(rdwr_p->m_has_cond) return 0;
			rdwr_p->m_cond = CH.m_p[i];
			rdwr_p->m_has_cond = 1;
		} else {
			if(rdwr_p->m_has_inc) return 0;
			rdwr_p->m_inc = CH.m_p[i];
			rdwr_p->m_has_inc = 1;
		}
	}

	return 1;
}

#endif
