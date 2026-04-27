#ifndef	aclattr_branchratio_h
#define aclattr_branchratio_h

#include <clang-c/Index.h>
#include <ae2f/Keys.h>
#include <util/log.h>
#include <util/wrd.h>
#include <string.h>

/** 
 * @param wr_ratio : aclspv_wrd_t[2] { true_weight , false_weight }
 * */
static enum CXChildVisitResult	s_aclattr_bratio(
		CXCursor h_cur,
		ae2f_unused CXCursor h_parent,
		CXClientData ae2f_restrict wr_ratio) {
	CXString	ATTR_KIND;
	const char*	NIDDLE;


	ATTR_KIND = clang_getCursorSpelling(h_cur);

	if(h_cur.kind == CXCursor_AnnotateAttr 
			&& (NIDDLE = strstr(clang_getCString(ATTR_KIND)
					, "aclspv_bratio"))) {
		unsigned	A0 = 0, A1 = 0;

		dbg_puts("s_attr_bratio::detected");
		sscanf(NIDDLE, "aclspv_bratio ( %u , %u )"
				, &A0
				, &A1
		      );

		clang_disposeString(ATTR_KIND);

		((aclspv_wrd_t* ae2f_restrict)(wr_ratio))[0] = A0;
		((aclspv_wrd_t* ae2f_restrict)(wr_ratio))[1] = A1;
		return CXChildVisit_Break;
	}

	return CXChildVisit_Continue;
}
#endif
