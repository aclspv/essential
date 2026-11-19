/** @file alias.h
 *
 * foreign attribute spellings accepted when the library is built with
 * `ACLSPV_ATTR_ALIASES`.
 *
 * clang exposes `__attribute__((reqd_work_group_size(...)))` only as an
 * unexposed attr with an empty spelling. the spelling and operands remain
 * in the cursor's source extent, so the alias is detected by tokenizing it.
 * */
#ifndef lib_attr_alias_h
#define lib_attr_alias_h

#include <aclspv.h>
#include <stdlib.h>
#include <string.h>

#if __ACLSPV_ATTR_ALIASES

static aclspv_bool_t s_attr_tok_is(
		CXTranslationUnit	c_tu,
		CXToken			c_tok,
		const char*		c_s
		) {
	CXString S = clang_getTokenSpelling(c_tu, c_tok);
	aclspv_bool_t EQ = strcmp(clang_getCString(S), c_s) == 0;
	clang_disposeString(S);
	return EQ;
}

static unsigned s_attr_tok_u32(
		CXTranslationUnit	c_tu,
		CXToken			c_tok
		) {
	CXString S = clang_getTokenSpelling(c_tu, c_tok);
	unsigned V = (unsigned)strtoul(clang_getCString(S), 0, 10);
	clang_disposeString(S);
	return V;
}

/** true when c_cur's source extent carries
 * `__attribute__((reqd_work_group_size(x, y, z)))`; the three operands are
 * written to the destinations. */
ae2f_unused static aclspv_bool_t s_attr_wrkgroup_alias(
		CXCursor	c_cur,
		unsigned*	wr_0,
		unsigned*	wr_1,
		unsigned*	wr_2
		) {
	CXTranslationUnit	TU = clang_Cursor_getTranslationUnit(c_cur);
	CXSourceRange		RNG;
	CXToken*		TOKS = 0;
	unsigned		N = 0;
	aclspv_bool_t		FOUND = 0;

	if(!TU)	return 0;

	RNG = clang_getCursorExtent(c_cur);
	clang_tokenize(TU, RNG, &TOKS, &N);

	if(N >= 8
			&& s_attr_tok_is(TU, TOKS[0], "reqd_work_group_size")
			&& s_attr_tok_is(TU, TOKS[1], "(")
			&& s_attr_tok_is(TU, TOKS[3], ",")
			&& s_attr_tok_is(TU, TOKS[5], ",")
			&& s_attr_tok_is(TU, TOKS[7], ")")) {
		*wr_0 = s_attr_tok_u32(TU, TOKS[2]);
		*wr_1 = s_attr_tok_u32(TU, TOKS[4]);
		*wr_2 = s_attr_tok_u32(TU, TOKS[6]);
		FOUND = 1;
	}

	if(TOKS)	clang_disposeTokens(TU, TOKS, N);
	return FOUND;
}

#endif
#endif
