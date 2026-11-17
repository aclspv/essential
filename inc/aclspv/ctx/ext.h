#ifndef	aclspv_ctx_ext_h

#include <aclspv/bool.h>
#include <aclspv/link.h>
#include <aclspv/common.h>
#include <aclspv/ctx/common.h>

#include <ae2f/Cxx/extern.h>
#include <ae2f/cc/fnpretty.h>

ae2f_extern ACLSPV_ABI_LINK ae2f_ccpure ae2f_noexcept aclspv_bool_t aclspv_chck_extension_from_ctx(
		const x_aclspv_ctx* const ae2f_restrict	rd_ctx,
		const char* ae2f_restrict const		c_ext
		);

ae2f_extern ACLSPV_ABI_LINK ae2f_noexcept aclspv_bool_t aclspv_add_extension_from_ctx(
		const h_aclspv_ctx_t			rd_ctx,
		const char* ae2f_restrict const		c_ext
		);

#endif
