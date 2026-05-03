#ifndef	aclattr_vecdecl_h
#define aclattr_vecdecl_h

#include <clang-c/Index.h>
#include <util/wrd.h>
#include <stdio.h>
#include <string.h>

static enum CXChildVisitResult aclattr_vecdecl_visit(
		CXCursor c_cur, 
		ae2f_unused CXCursor c_par,
		CXClientData ae2f_restrict wr_vecnum
		) { if(c_cur.kind == CXCursor_AnnotateAttr) {
	const CXString ATTR = clang_getCursorSpelling(c_cur);

	if(strstr(clang_getCString(ATTR), "aclspv_vecdecl")) {
		unsigned RET = 0;
		(void)sscanf(
				clang_getCString(ATTR)
				, "aclspv_vecdecl ( %u )"
				, &RET
				);

		*((aclspv_wrd_t*)wr_vecnum) = RET;
	}

	clang_disposeString(ATTR);

	return CXChildVisit_Break;
} else	return CXChildVisit_Continue;
}

ae2f_inline static aclspv_wrd_t	aclattr_vecdecl(aclspv_wrd_t c_num_default, CXCursor c_root) {
	aclspv_wrd_t	A = c_num_default;
	clang_visitChildren(c_root, aclattr_vecdecl_visit, &A);
	return A;
}

#endif
