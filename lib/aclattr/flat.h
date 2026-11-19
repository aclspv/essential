/** @file flat.h */

#ifndef lib_attr_flat_h
#define lib_attr_flat_h

#include <ae2f/Keys.h>
#include <string.h>
#include <aclspv.h>

/**
 * detect a bare `aclspv_flat` annotation on the cursor.
 *
 * writes 1 to *wr_flat when found and stops the visit; leaves the
 * destination untouched otherwise.
 *
 * @fn static enum CXChildVisitResult s_attr_flat(
 *     CXCursor h_cur,
 *     CXCursor h_parent,
 *     aclspv_wrd_t* ae2f_restrict wr_flat
 * )
 * */
static enum CXChildVisitResult	s_attr_flat(
		CXCursor h_cur,
		ae2f_unused CXCursor h_parent,
		CXClientData ae2f_restrict wr_flat) {
	CXString TEXT;

	unless(h_cur.kind == CXCursor_AnnotateAttr)
		return CXChildVisit_Recurse;

	TEXT = clang_getCursorSpelling(h_cur);

	unless(strstr(TEXT.data, "aclspv_flat")) {
#if __ACLSPV_ATTR_ALIASES
		/** `annotate("flat")` */
		if(!strcmp(TEXT.data, "flat")) {
			*((aclspv_wrd_t* ae2f_restrict)wr_flat) = 1;
			clang_disposeString(TEXT);
			return CXChildVisit_Break;
		}
#endif
		clang_disposeString(TEXT);
		return CXChildVisit_Continue;
	}

	*((aclspv_wrd_t* ae2f_restrict)wr_flat) = 1;

	clang_disposeString(TEXT);
	return CXChildVisit_Break;
}

#endif
