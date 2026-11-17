#ifndef	aclspv_ctx_init_fini_h
#define aclspv_ctx_init_fini_h

#include "./common.h"

#include <aclspv/link.h>
#include <aclspv/usrcfg.h>
#include <ae2f/Cxx/extern.h>

#ifdef	aclspv_h

#include <aclspv/enum/err.h>

ae2f_extern ACLSPV_ABI_LINK aclspv_compile_t aclspv_init_ctx(
		const h_aclspv_ctx_t	i_ctx,
		const aclspv_usrcfg_t*	ae2f_restrict	rd_usrcfg
		);

/** 
 * @brief	free the resources of `d_ctx`.
 * */
ae2f_extern ACLSPV_ABI_LINK void aclspv_fini_ctx(
		const h_aclspv_ctx_t	d_ctx
		);

#endif

#include <ae2f/cc/fnpretty.h>

ae2f_extern ACLSPV_ABI_LINK 
void		aclspv_ctx_free(x_aclspv_ctx* d);

ae2f_extern ACLSPV_ABI_LINK ae2f_retnew1(aclspv_ctx_free)
x_aclspv_ctx*	aclspv_ctx_make(const aclspv_usrcfg_t* ae2f_restrict rd_usrcfg);

#endif
