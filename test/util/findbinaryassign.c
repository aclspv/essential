#include <clang-c/CXString.h>
#include <clang-c/Index.h>
#include <ae2f/cc.h>
#include <stdio.h>

#define	SOURCE	\
	"int AAA(void);"	\
	"int AAA(void) {"	\
	"	int A = 3;"	\
	"	int B = 3;"	\
	"	int C = B + A += B;"		\
	"}"

static enum CXChildVisitResult	s_find_assign_add(
		CXCursor c_cur, 
		ae2f_unused CXCursor c_par, 
		ae2f_unused CXClientData unused) {
	const CXString		SPL		= clang_getCursorSpelling(c_cur);
	const CXString		KNDSPL		= clang_getCursorKindSpelling(clang_getCursorKind(c_cur));
	const char* const	SPL_S		= clang_getCString(SPL);
	const char* const	KNDSPL_S	= clang_getCString(KNDSPL);
	const char* MSG				= "SKIP";
	enum CXChildVisitResult RET		= CXChildVisit_Recurse;

	puts("===================================");
	puts(SPL_S);
	puts(KNDSPL_S);

	if(c_cur.kind != CXCursor_CompoundAssignOperator) {
		goto END;
	}

#if 1
	if(clang_getCursorBinaryOperatorKind(c_cur) != CXBinaryOperator_AddAssign)
		return CXChildVisit_Recurse;
#endif

	MSG = "FOUND";
	RET = CXChildVisit_Break;

END:
	puts(MSG);
	clang_disposeString(SPL);
	clang_disposeString(KNDSPL);
	return RET;
}

int main(void);
int main(void) {
	const CXIndex CXIDX		= clang_createIndex(0, 0);
	CXTranslationUnit	CXTU	= 0;
	CXCursor		CXROOTCUR;
	struct CXUnsavedFile	UFILE;

	UFILE.Filename = "main.c";
	UFILE.Length = sizeof(SOURCE) - 1;
	UFILE.Contents = SOURCE;

	puts("Hello, World!");

	clang_parseTranslationUnit2(
			CXIDX, "main.c"
			, 0
			, 0
			, &UFILE, 1
			, CXTranslationUnit_None, &CXTU
			);

	CXROOTCUR = clang_getTranslationUnitCursor(CXTU);

	clang_visitChildren(CXROOTCUR, s_find_assign_add, 0);

	clang_disposeIndex(CXIDX);
	clang_disposeTranslationUnit(CXTU);

	return 0;
}
