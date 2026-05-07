#ifndef aclattr_rdattr_cc
#define aclattr_rdattr_cc

#include <ae2f/pat/macrofunc.h>
#include <clang-c/Index.h>
#include <string.h>

ae2f_MAC((L, L_NDL, )) aclattr_rdattr(
		const CXCursor c_cur,
		const char* const ae2f_restrict rd_ndl,
		enum CXChildVisitResult&	ret,
		const enum CXChildVisitResult	c_on_found,
		const enum CXChildVisitResult	c_on_not_found,
		int __scanf
		) {
	CXString			L$$ATTRSPL	= clang_getCursorSpelling(c_cur);
	const char* ae2f_restrict	L_NDL		= clang_getCString(L$$ATTRSPL) ? 
		strstr(
				clang_getCString(L$$ATTRSPL)
				, rd_ndl) : 0;

	if(L_NDL) {
		(void)__scanf;
		ret	= c_on_found;
	} else {
		ret	= c_on_not_found;
	}

	clang_disposeString(L$$ATTRSPL);
}

#endif
