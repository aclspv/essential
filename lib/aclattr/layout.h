#ifndef	aclattr_layout_h
#define aclattr_layout_h

#include <aclty/layout.h>
#include <aclctx/err.h>

#include <string.h>
#include <stdio.h>


typedef struct {
	aclty_layout_t	m_layout;
	h_aclspv_ctx_t	m_ctx;
} s_aclattr_layout_t;

static enum CXChildVisitResult s_aclattr_layout(CXCursor c, ae2f_unused CXCursor _, CXClientData h) {
	s_aclattr_layout_t* ae2f_restrict ret = h;

	CXString TEXT;
	enum CXChildVisitResult RES = CXChildVisit_Break;
	const char* NEEDLE;
	unsigned ret_un;
	aclty_layout_t	layout;

	unless(c.kind == CXCursor_AnnotateAttr)
		return CXChildVisit_Recurse;

	TEXT = clang_getCursorSpelling(c);

	unless(NEEDLE = strstr(TEXT.data, "aclspv_layout")) {
		RES = CXChildVisit_Continue;
		goto LBL_FINI;
	}

	sscanf(NEEDLE, "aclspv_layout ( %u )", &ret_un);
	layout = (aclty_layout_t)ret_un;

	ae2f_ifnezerr_strict(layout != ret->m_layout && ret->m_layout != ACLSPV_LAYOUT_MAX) {
		RES = CXChildVisit_Break;
		_aclctx_err( ret->m_ctx, ACLSPV_COMPILE_UNHANDLED);
		goto LBL_FINI;
	}

	ret->m_layout = layout;

LBL_FINI:
	clang_disposeString(TEXT);
	return RES;
}

/** can emit an error */
static aclspv_layout_t aclattr_get_layout(CXCursor c, CXType c_ty, h_aclspv_ctx_t h_ctx) {
	s_aclattr_layout_t s;

	s.m_ctx		= h_ctx;
	s.m_layout	= ACLSPV_LAYOUT_MAX;

	clang_visitChildren(c, s_aclattr_layout, &s);
	clang_visitChildren(clang_getCanonicalCursor(c), s_aclattr_layout, &s);
	clang_visitChildren(clang_getTypeDeclaration(c_ty), s_aclattr_layout, &s);

	return s.m_layout == ACLSPV_LAYOUT_MAX ? h_ctx->m_usrcfg.m_layout_default : s.m_layout;
}

#endif
