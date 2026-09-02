#include "./aclmem.h"

aclspv_bool_t aclmem_calloc(x_aclspv_vec* ae2f_restrict	h,
		aclspv_wrd_t			c_sz
		) {
	aclspv_bool_t		ret;
	ae2f_ifezerr(h)		return 1;

	ae2f_ifneznrm(h->m_sz_real >= c_sz) {
		h->m_sz = c_sz;
		ret	= 0;
	} else {
		void* ae2f_restrict	base;
		void* ae2f_restrict	mem;
		aclspv_wrd_t		sz_real;

		ret = !(base = h->m_ctx->m_usrcfg.m_malloc(
					c_sz
					, &sz_real
					, &mem
					, h->m_ctx->m_usrcfg.m_usrdata
					));

		h->m_ctx->m_usrcfg.m_free(h->m_base, h->m_sz_real
				, h->m_ctx->m_usrcfg.m_usrdata);

		ae2f_ifnezerr_strict(ret) {
			return 1;
		}

		h->m_sz		= c_sz;
		h->m_sz_real	= sz_real;
		h->m_base	= base;
		h->m_p		= mem;
	}

	return ret;
}

aclspv_bool_t aclmem_cgrow(x_aclspv_vec* ae2f_restrict	h,
		aclspv_wrd_t			c_sz
		) {
	aclspv_bool_t		ret;
	ae2f_ifezerr(h)		return 1;

	ae2f_ifezerr(h->m_base)
		return aclmem_calloc(h, c_sz);

	ae2f_ifneznrm(h->m_sz_real >= c_sz) {
		h->m_sz = c_sz;
		ret	= 0;
	} else {
		void* ae2f_restrict	base;
		void* ae2f_restrict	mem;
		aclspv_wrd_t		sz_real;

		ret = !(base = h->m_ctx->m_usrcfg.m_malloc(
					c_sz
					, &sz_real
					, &mem
					, h->m_ctx->m_usrcfg.m_usrdata
					));

		ae2f_ifnezerr_strict(ret) {
			return 1;
		}

		aclmemcpy(mem, h->m_p, h->m_sz);

		h->m_ctx->m_usrcfg.m_free(h->m_base, h->m_sz_real
				, h->m_ctx->m_usrcfg.m_usrdata);

		h->m_sz		= c_sz;
		h->m_sz_real	= sz_real;
		h->m_base	= base;
		h->m_p		= mem;
	}

	return ret;
}

void		aclmem_init(
		x_aclspv_vec* ae2f_restrict	fini,
		h_aclspv_ctx_t			h_ctx
		) {
	ae2f_ifezerr(fini)	return;

	unless(h_ctx) {
		if(fini->m_base) fini->m_ctx->m_usrcfg.m_free(
				fini->m_base
				, fini->m_sz_real
				, fini->m_ctx->m_usrcfg.m_usrdata
				);
	}

	fini->m_base	= 0;
	fini->m_p	= 0;
	fini->m_sz	= 0;
	fini->m_sz_real	= 0;
	fini->m_ctx	= h_ctx;
}

void		aclmemcpy(void* ae2f_restrict wr, const void* ae2f_restrict rd, aclspv_wrd_t c) {
	char* ae2f_restrict		wr_ = wr;
	const char* ae2f_restrict	rd_ = rd;

	while(c --> 0) {
		*wr_++ = *rd_++;
	}
}



