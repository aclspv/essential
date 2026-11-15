#ifndef	aclmem_h
#define aclmem_h

#include <aclspv.h>

/** 
 * when zero it succeed. 
 * does not set h_ctx.
 * */
aclspv_bool_t	aclmem_cgrow(
		x_aclspv_vec* ae2f_restrict	h,
		aclspv_wrd_t			c_sz
		);

/** 
 * when zero it succeed. 
 * does not set h_ctx.
 * */
aclspv_bool_t	aclmem_calloc(
		x_aclspv_vec* ae2f_restrict	h,
		aclspv_wrd_t			c_sz
		);

/** when zero it succeed. */
ae2f_inline_f static aclspv_bool_t	s_aclmem_cgrow2(
		x_aclspv_vec* ae2f_restrict	h,
		aclspv_wrd_t			c_elmc,
		aclspv_wrd_t			c_elml,
		enum ACLSPV_COMPILE_		c_err_in_case
		) {
	ae2f_ifnezerr_strict(
			c_elmc * c_elml < c_elmc || 
			c_elmc * c_elml < c_elml || 
			aclmem_cgrow(h, c_elmc * c_elml)) {
		h->m_ctx->m_err = (aclspv_compile_t)c_err_in_case;
		return 1;
	}

	return 0;
}

/** 1 on error. */
ae2f_inline_f static aclspv_bool_t	s_aclmem_calloc2(
		x_aclspv_vec* ae2f_restrict	h,
		aclspv_wrd_t			c_elmc,
		aclspv_wrd_t			c_elml,
		enum ACLSPV_COMPILE_		c_err_in_case
		) {
	ae2f_ifnezerr_strict(
			c_elmc * c_elml < c_elmc ||
			c_elmc * c_elml < c_elml ||
			aclmem_calloc(h, c_elmc * c_elml)) {
		h->m_ctx->m_err = (aclspv_compile_t)c_err_in_case;
		return 1;
	}

	return 0;
}

#include "./aclmem/ptr.h"

#ifndef	aclmem_cvoid_aclmem_ptr
#define aclmem_cvoid_aclmem_ptr

typedef const void aclmem_cvoid;
aclmem_ptr(aclmem_cvoid);

#endif

ae2f_ccconst ae2f_inline_f static aclspv_wrd_t aclmemremaddr(const void* c_p, aclspv_wrd_t c_a)
{
	const aclmem_cvoid_aclmem_ptr_t c = aclmem_cvoid_aclmem_ptr_to_wrd(c_p);
	return c.m_a[0] & ((c_a) - 1u);
}

void aclmem_init(x_aclspv_vec* ae2f_restrict fini, h_aclspv_ctx_t h_ctx);

void aclmemcpy(void* ae2f_restrict wr, const void* ae2f_restrict rd, aclspv_wrd_t c);
void aclmemzero(void* ae2f_restrict wr, aclspv_wrd_t c);

/** when zeroed, this is fine. */
#define		aclmem_fini(init)	aclmem_init(init, 0)

#endif
