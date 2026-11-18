#ifndef	aclemit_fndecl_sym_h
#define	aclemit_fndecl_sym_h

#include <aclspv.h>
#include <aclsym/fn.h>

#include <string.h>

/**
 * emit `OpName %id "<declared name>"` for an emitted symbol, when
 * ACLSPV_CTX_FLAG0_SYMBOL asks for it.
 *
 * the name is a debugging aid: calls and linkage resolve by id, and an import
 * already carries its name in the LinkageAttributes decoration. the flag is
 * what keeps the module free of names when they are not wanted.
 *
 * the symbol must have its m_id already assigned.
 * */
static aclspv_compile_t aclemit_fndecl_sym_name(
		const aclsym_fn_t*	c_sym,
		h_aclspv_ctx_t		h_ctx
		) {
	const char*	c_name;
	aclspv_wrd_t	c_len;

	if(!(h_ctx->m_usrcfg.m_flag0 & ACLSPV_CTX_FLAG0_SYMBOL))
		return h_ctx->m_err;

	c_name = aclsym_fn_name(c_sym, h_ctx);

	/** the cache always stores a name; an empty one names nothing. */
	unless(c_name)
		return h_ctx->m_err;

	c_len = (aclspv_wrd_t)strlen(c_name);

	ae2f_ifezerr(aclspv_cfg_symbol(c_name, c_len, c_sym->m_id, h_ctx))
		return h_ctx->m_err = (
				h_ctx->m_err
				? h_ctx->m_err
				: ACLSPV_COMPILE_ALLOC_FAILED);

	return h_ctx->m_err;
}

#endif
