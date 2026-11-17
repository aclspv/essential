#ifndef	aclspv_ctx_extinst_h
#define aclspv_ctx_extinst_h

#include "./common.h"

#include <aclspv/wrd.h>
#include <aclspv/id.h>
#include <aclspv/link.h>

#include <ae2f/cc/fnpretty.h>
#include <ae2f/Cxx/extern.h>

/** extended instruction */
typedef aclspv_id_t aclspv_extinst_t;

/** 
 * check an extension instruction import
 * 0 on none has found.
 * */
ae2f_extern ACLSPV_ABI_LINK ae2f_ccpure ae2f_noexcept aclspv_extinst_t aclspv_chck_extinst(
		const char* ae2f_restrict rd,
		h_aclspv_ctx_t h_ctx
		);

/**
 * import new instruction
 * 0 on failure
 * */
ae2f_extern ACLSPV_ABI_LINK ae2f_noexcept aclspv_extinst_t aclspv_add_extinst_from_ctx(
		const h_aclspv_ctx_t h_ctx,
		const char* ae2f_restrict const		rd
		);

#endif
