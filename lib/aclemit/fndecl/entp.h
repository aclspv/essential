#ifndef	aclemit_fndecl_entp_h
#define aclemit_fndecl_entp_h


#include <aclspv.h>
enum CXChildVisitResult aclemit_fndecl_entp(CXCursor h_cur, CXCursor h_parent, CXClientData h_ctx);

/**
 * true when the entry point `c_entp` declares is already in this module: an
 * OpEntryPoint and a body were emitted for a function of the same name.
 *
 * an entry point is not a call target and carries no function symbol, so this
 * list is the only record of it. a context outlives one translation unit and
 * the build phase re-walks the translation unit the compile phase saw, so
 * without this test the second walk would push a second OpEntryPoint for one
 * function.
 * */
aclspv_bool_t aclemit_fndecl_entp_exists(
		CXCursor		c_entp,
		h_aclspv_ctx_t		h_ctx
		);

#endif
