/** @file fnctrl.h
 *
 * the `OpFunction` control mask, read from the declaration's attributes at
 * emission time. no pre-pass, no stored state: the mask is a property of the
 * declaration and is only needed when the instruction is written.
 *
 * `always_inline` -> Inline, `noinline` -> DontInline. +
 * the C `inline` keyword is not an attribute; where it is a keyword, clang
 * reports it through clang_Cursor_isFunctionInlined().
 * */

#ifndef	aclattr_fnctrl_h
#define	aclattr_fnctrl_h

#include <aclspv.h>
#include <ae2f/cc.h>

#include <string.h>

/** the spelling of a bare attribute cursor equals c_name.
 *
 * libclang exposes `always_inline` and `noinline` as
 * CXCursor_UnexposedAttr with an empty spelling; the name survives only as
 * the token under the cursor's extent. `__attribute__((__always_inline__))`
 * keeps its underscores, so both spellings are probed. */
static aclspv_bool_t aclattr_attr_named(CXCursor c_attr, const char* c_name) {
	const CXTranslationUnit	TU	= clang_Cursor_getTranslationUnit(c_attr);
	const CXSourceRange	RANGE	= clang_getCursorExtent(c_attr);
	CXToken*		p_tok	= 0;
	unsigned		c_tok	= 0;
	aclspv_bool_t		c_hit	= 0;

	if(!TU)
		return 0;

	clang_tokenize(TU, RANGE, &p_tok, &c_tok);

	if(c_tok && p_tok) {
		CXString	SPELL		= clang_getTokenSpelling(TU, p_tok[0]);
		const char*	c_spell		= clang_getCString(SPELL);

		c_hit = (aclspv_bool_t)(c_spell && !strcmp(c_spell, c_name));
		clang_disposeString(SPELL);
	}

	if(p_tok)
		clang_disposeTokens(TU, p_tok, c_tok);

	return c_hit;
}

struct aclattr_fnctrl {
	aclspv_bool_t	m_inline;
	aclspv_bool_t	m_noinline;
};

static enum CXChildVisitResult aclattr_fnctrl_visit(
		CXCursor		c_cur,
		ae2f_unused CXCursor	c_par,
		CXClientData		rdwr_ctrl
		) {
	struct aclattr_fnctrl* ae2f_restrict p = rdwr_ctrl;

	/** the body is a child too: never descend into it. */
	if(c_cur.kind != CXCursor_UnexposedAttr)
		return CXChildVisit_Continue;

	if(aclattr_attr_named(c_cur, "always_inline")
			|| aclattr_attr_named(c_cur, "__always_inline__"))
		p->m_inline = 1;
	else if(aclattr_attr_named(c_cur, "noinline")
			|| aclattr_attr_named(c_cur, "__noinline__"))
		p->m_noinline = 1;

	return CXChildVisit_Continue;
}


ae2f_unused static aclspv_bool_t aclattr_is_noinline(CXCursor c_fn) {
	struct aclattr_fnctrl r;

	r.m_inline = 0;
	r.m_noinline = 0;
	clang_visitChildren(c_fn, aclattr_fnctrl_visit, &r);
	return r.m_noinline;
}

ae2f_unused static aclspv_bool_t aclattr_is_always_inline(CXCursor c_fn) {
	struct aclattr_fnctrl r;

	r.m_inline = 0;
	r.m_noinline = 0;
	clang_visitChildren(c_fn, aclattr_fnctrl_visit, &r);
	return (aclspv_bool_t)(r.m_inline && !r.m_noinline);
}

/** the OpFunction control mask for c_fn. `noinline` wins when both are
 * present: refusing to inline is the safer promise. */
ae2f_unused static aclspv_wrd_t aclattr_get_fnctrl(CXCursor c_fn) {
	struct aclattr_fnctrl r;

	r.m_inline	= 0;
	r.m_noinline	= 0;

	clang_visitChildren(c_fn, aclattr_fnctrl_visit, &r);

	if(r.m_noinline)
		return (aclspv_wrd_t)SpvFunctionControlDontInlineMask;

	if(r.m_inline || clang_Cursor_isFunctionInlined(c_fn))
		return (aclspv_wrd_t)SpvFunctionControlInlineMask;

	return (aclspv_wrd_t)SpvFunctionControlMaskNone;
}

#endif
