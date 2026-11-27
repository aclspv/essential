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

/** @file pure.h
 *
 * `__attribute__((pure))` and `__attribute__((const))`: the effect promise a
 * function makes about itself.
 *
 * - pure:  no observable side effects; it may read state it does not own.
 * - const: no observable side effects and no reads beyond its arguments
 *          (strictly stronger than pure; const implies pure).
 *
 * a body in this translation unit can be analysed, but a body-less callee
 * cannot: the declaration is all a call site gets. these two attributes are
 * how such a callee describes its effects, and why a call to an imported
 * function is analysable at all.
 * */

#ifndef	aclattr_pure_h
#define	aclattr_pure_h

#include <aclspv.h>
#include <ae2f/c90/stdint.h>

typedef struct {
	aclspv_bool_t	m_is_pure;
	aclspv_bool_t	m_is_const;
} aclattr_pure_t;

/** collect the declared purity on rdwr_pure.
 *
 * both attributes may appear on a declaration, a definition, or be split
 * across redeclarations: the caller accumulates, never overwrites.
 * */
static enum CXChildVisitResult aclattr_pure(
		CXCursor		c_cur,
		ae2f_unused CXCursor	c_par,
		CXClientData		rdwr_pure
		) {
	aclattr_pure_t* ae2f_restrict p = rdwr_pure;

	switch((uintmax_t)c_cur.kind) {
		default:
			return CXChildVisit_Recurse;

		case CXCursor_PureAttr:
			p->m_is_pure = 1;
			return CXChildVisit_Continue;

		case CXCursor_ConstAttr:
			/** const implies pure. */
			p->m_is_pure	= 1;
			p->m_is_const	= 1;
			return CXChildVisit_Continue;
	}
}

/** the declared purity of c_fn: both zero when it carries neither attribute. */
static aclattr_pure_t aclattr_get_pure(CXCursor c_fn) {
	aclattr_pure_t r;

	r.m_is_pure	= 0;
	r.m_is_const	= 0;

	clang_visitChildren(c_fn, aclattr_pure, &r);

	return r;
}

#endif
