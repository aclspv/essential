#ifndef	aclutil_visitfromroot_h
#define aclutil_visitfromroot_h

#include <ae2f/cc.h>
#include <ae2f/cgoto.h>
#include <clang-c/Index.h>

ae2f_always_inline static enum CXChildVisitResult aclutil_visitfromroot(
		CXCursor	c_cursor,
		CXCursorVisitor	fn_visit,
		CXClientData	h_clientdata
		) {
	enum CXChildVisitResult R = fn_visit(c_cursor, c_cursor, h_clientdata);

	if(R == CXChildVisit_Recurse) {
		R = (enum CXChildVisitResult) clang_visitChildren(c_cursor, fn_visit, h_clientdata);
	}

	return R;
}

#endif
