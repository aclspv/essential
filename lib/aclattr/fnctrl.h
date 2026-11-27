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

/** @file fnctrl.h
 *
 * how a function declaration asks to be inlined. two different things are
 * read here and they must never be conflated:
 *
 * - the CALL-SITE policy, which decides whether the call is carved into the
 *   caller or emitted as `OpFunctionCall`:
 *
 *   `always_inline` (also `__always_inline__`)	-> always carve
 *   `__forceinline__` (MSVC alias of `always_inline`)	-> always carve
 *   the declaration's `inline` keyword			-> carve iff
 *					   ACLSPV_CTX_FLAG0_INLINE_BY_DEFAULT
 *   anything else / `noinline`			-> call
 *
 * - the `OpFunction` control mask, which is what the module tells a
 *   downstream inliner about the declaration we emitted:
 *
 *   `noinline`		-> DontInline
 *   everything else	-> Inline
 *
 * the mask is therefore never `None` for a normal function: the emitted
 * function is always inlineable, and only an explicit `noinline` opts out.
 * */

#ifndef	aclattr_fnctrl_h
#define	aclattr_fnctrl_h

#include <aclspv.h>
#include <ae2f/cc.h>

#include <string.h>

/** does the attribute cursor name c_name?
 *
 * libclang reports a typed attribute with its own name, and the generic
 * CXCursor_UnexposedAttr form with an empty spelling — the name then lives
 * in the words under the cursor's extent, which libclang already lexed into
 * the translation unit. re-lexing that range is a buffer lookup, not a file
 * read, so an unsaved source works. `__attribute__((__always_inline__))`
 * keeps its underscores, so those spellings are probed by the caller. */
static aclspv_bool_t aclattr_attr_named(CXCursor c_attr, const char* c_name) {
	const CXTranslationUnit	TU	= clang_Cursor_getTranslationUnit(c_attr);
	const CXSourceRange	RANGE	= clang_getCursorExtent(c_attr);
	CXToken*		p_tok	= 0;
	unsigned		c_tok	= 0;
	aclspv_bool_t		c_hit	= 0;

	if(TU)
		clang_tokenize(TU, RANGE, &p_tok, &c_tok);

	if(!c_tok || !p_tok) {
		/** a typed attribute: its spelling is the name. */
		CXString	SPELL	= clang_getCursorSpelling(c_attr);
		const char*	c_spell	= clang_getCString(SPELL);

		c_hit = (aclspv_bool_t)(c_spell && c_spell[0u]
				&& !strcmp(c_spell, c_name));
		clang_disposeString(SPELL);
	}
	else {
		/** the last word is the name: `always_inline`, or the tail of
		 * `ns::always_inline`. */
		CXString	SPELL	= clang_getTokenSpelling(TU, p_tok[c_tok - 1u]);
		const char*	c_spell	= clang_getCString(SPELL);

		c_hit = (aclspv_bool_t)(c_spell && !strcmp(c_spell, c_name));
		clang_disposeString(SPELL);
	}

	if(p_tok)
		clang_disposeTokens(TU, p_tok, c_tok);

	return c_hit;
}

struct aclattr_fnctrl {
	aclspv_bool_t	m_always;
	aclspv_bool_t	m_noinline;
};

static enum CXChildVisitResult aclattr_fnctrl_visit(
		CXCursor		c_cur,
		ae2f_unused CXCursor	c_par,
		CXClientData		rdwr_ctrl
		) {
	struct aclattr_fnctrl* ae2f_restrict p = rdwr_ctrl;

	/** the body is a child too: never descend into it. every attribute
	 * libclang does not type is a CXCursor_UnexposedAttr. */
	if(c_cur.kind != CXCursor_UnexposedAttr)
		return CXChildVisit_Continue;

	/** `always_inline` / `__always_inline__` / `__forceinline__`, and the
	 * namespace-qualified spellings, all land on the same attribute. */
	if(aclattr_attr_named(c_cur, "always_inline")
			|| aclattr_attr_named(c_cur, "__always_inline__")
			|| aclattr_attr_named(c_cur, "forceinline")
			|| aclattr_attr_named(c_cur, "__forceinline__")
			|| aclattr_attr_named(c_cur, "__forceinline"))
		p->m_always = 1;
	else if(aclattr_attr_named(c_cur, "noinline")
			|| aclattr_attr_named(c_cur, "__noinline__"))
		p->m_noinline = 1;

	return CXChildVisit_Continue;
}

static void aclattr_fnctrl_read(CXCursor c_fn, struct aclattr_fnctrl* wr) {
	wr->m_always	= 0;
	wr->m_noinline	= 0;
	clang_visitChildren(c_fn, aclattr_fnctrl_visit, wr);
}

/** the declaration carries `always_inline` (or its MSVC alias) and not
 * `noinline`. such a callee MUST be carved at the call site. */
ae2f_unused static aclspv_bool_t aclattr_is_always_inline(CXCursor c_fn) {
	struct aclattr_fnctrl r;

	aclattr_fnctrl_read(c_fn, &r);
	return (aclspv_bool_t)(r.m_always && !r.m_noinline);
}

/** the declaration carries `noinline`. */
ae2f_unused static aclspv_bool_t aclattr_is_noinline(CXCursor c_fn) {
	struct aclattr_fnctrl r;

	aclattr_fnctrl_read(c_fn, &r);
	return r.m_noinline;
}

/** the declaration's `inline` key word (or an attribute that implies it).
 * this is NOT the C++ class-body implicit inline: a member function defined
 * in its class is implicitly inline in C++, but that is a linkage property
 * with no attribute attached, and inlining it at every call site would be
 * the opposite of what the source asked for. */
ae2f_unused static aclspv_bool_t aclattr_is_kw_inline(CXCursor c_fn) {
	struct aclattr_fnctrl r;

	aclattr_fnctrl_read(c_fn, &r);

	/** `noinline` / `always_inline` are decided by their own rules; the
	 * `inline` key word only matters on its own. */
	if(r.m_always || r.m_noinline)
		return 0;

	return (aclspv_bool_t)clang_Cursor_isFunctionInlined(c_fn);
}

/** would an inlined copy of this callee's body be carved into the caller?
 *
 * this is the call-site half of the policy without the per-call ordering
 * (whether the callee must be emitted first is another question):
 * `always_inline` always, the `inline` key word only under
 * ACLSPV_CTX_FLAG0_INLINE_BY_DEFAULT, `noinline` never. */
ae2f_unused static aclspv_bool_t aclattr_wants_inlining(
		CXCursor		c_fn,
		h_aclspv_ctx_t		h_ctx
		) {
	struct aclattr_fnctrl r;

	aclattr_fnctrl_read(c_fn, &r);

	if(r.m_noinline)
		return 0;

	if(r.m_always)
		return 1;

	return (aclspv_bool_t)(clang_Cursor_isFunctionInlined(c_fn)
			&& (h_ctx->m_usrcfg.m_flag0
				& ACLSPV_CTX_FLAG0_INLINE_BY_DEFAULT));
}

/** the `OpFunction` control mask for c_fn (a non-entry-point function).
 *
 * only an explicit `noinline` opts out of inlining downstream: without it
 * the module declares the function inlineable. entry points are emitted by
 * aclemit_fndecl_entp and never go through here. */
ae2f_unused static aclspv_wrd_t aclattr_get_fnctrl(CXCursor c_fn) {
	struct aclattr_fnctrl r;

	aclattr_fnctrl_read(c_fn, &r);

	if(r.m_noinline)
		return (aclspv_wrd_t)SpvFunctionControlDontInlineMask;

	return (aclspv_wrd_t)SpvFunctionControlInlineMask;
}

#endif
