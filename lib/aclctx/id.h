#ifndef aclctx_id_h
#define aclctx_id_h

#include <aclspv.h>



#if	__ACLSPV_ATOMIC_ID
#include <c89atomic.h>
#endif


ae2f_inline_f static aclspv_id_t aclctx_newid(h_aclspv_ctx_t h) {
#if	__ACLSPV_ATOMIC_ID
	aclspv_id_t new_ = c89atomic_fetch_add_32(&h->m_id, 1);
#else
	aclspv_id_t new_ = h->m_id++;
#endif

#if	1
	ae2f_ifnezerr_strict(new_ <= new_ - 1)
		return 0;
#endif

	return new_;
}

#endif
