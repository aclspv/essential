/** @file btcast.h */

#ifndef	attr_btcast_h
#define attr_btcast_h

#include <ae2f/cc.h>
#include <string.h>
#include <util/log.h>

#include <aclspv.h>

/** 
 * @fn static enum CXChildVisitResult attr_execmodel(CXCursor h_cur, CXCursor h_parent, aclspv_bool_t* ae2f_restrict wr_is_btcast)
 * */
static enum CXChildVisitResult aclattr_btcast(
		CXCursor h_cur, 
		ae2f_unused CXCursor h_parent, 
		CXClientData ae2f_restrict wr_is_btcast) {
	CXString	ATTR_KIND;
	const char*	NIDDLE;


	ATTR_KIND = clang_getCursorSpelling(h_cur);

	dbg_prefix(a);
	dbg_call(printf, ("\t%s\n", clang_getCString(ATTR_KIND)));

	if(h_cur.kind == CXCursor_AnnotateAttr && (NIDDLE = strstr(clang_getCString(ATTR_KIND), "aclspv_btcast"))) {
		dbg_puts("s_attr_btcast::detected");
		*((aclspv_bool_t* ae2f_restrict)wr_is_btcast) = 1;

		clang_disposeString(ATTR_KIND);
		return CXChildVisit_Break;
	}

	clang_disposeString(ATTR_KIND);


	return CXChildVisit_Recurse;
}

#endif
