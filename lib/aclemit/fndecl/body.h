#ifndef aclemit_fndecl_entp_body_h
#define aclemit_fndecl_entp_body_h

#include <aclspv.h>

enum CXChildVisitResult	aclemit_fndecl_bodycfg(
		CXCursor h_cur,
		h_aclspv_ctx_t	h_ctx,
		aclspv_bool_t* ae2f_restrict	wr_is_const,
		aclspv_bool_t* ae2f_restrict	wr_is_pure
		);

#endif
