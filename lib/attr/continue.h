#ifndef	aclattr_continue_h
#define	aclattr_continue_h

#include <aclspv.h>
#include <ae2f/c90/StdInt/enum.h>

/** @returns aclspv_bool_t */
static enum CXChildVisitResult aclattr_continue(
		CXCursor h_cur, 
		ae2f_unused CXCursor h_parent, 
		CXClientData ae2f_restrict r_does_continue_exist) {
	switch((uint_leastenum_t)h_cur.kind) {
		case CXCursor_ForStmt:
		case CXCursor_WhileStmt:
		case CXCursor_DoStmt:
		case CXCursor_CXXForRangeStmt:
			return CXChildVisit_Continue;

		default:
			return CXChildVisit_Recurse;

		case CXCursor_ContinueStmt:
			*((aclspv_bool_t* ae2f_restrict)r_does_continue_exist) = 1;
			return CXChildVisit_Break;
	}
}

#endif
