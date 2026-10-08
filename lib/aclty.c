
#include "./aclty.h"
#include <ae2f/pat/dbg.h>

#include <util/iddef.h>


ae2f_ccpure	aclty_t* aclty_find(const aclspv_id_t c, const h_aclspv_ctx_t h_ctx) {
	ae2fpat_assertassume_strict(h_ctx);

	ae2f_ifneznrm(c) {
		size_t	C = (size_t)h_ctx->m_num_type_uniques;
		aclty_t* ae2f_restrict P =  (aclty_t* ae2f_restrict)h_ctx->m_type_uniques.m_p;
		while( C --> 0 && (P++)->m_type_id != c) {}
		return ae2f_expected_strict(
				(C) < (size_t)(h_ctx->m_num_type_uniques)
				) ? P - 1 : 0;
	}

	return 0;
}

aclconst_specsz	aclty_getsize(const aclspv_id_t c, const h_aclspv_ctx_t h_ctx) {
	aclconst_specsz	r;
	ae2f_ifneznrm_strict(c >= ACLSPV_ID_END) {
		aclty_t* ae2f_restrict cxtp = aclty_find(c, h_ctx);
		ae2f_ifezerr(cxtp) {
			r.m_id = 0;
			r.m_lit.m_wrd = 0;
			return r;
		}
		return cxtp->m_sizeof;
	}

	{
		r.m_id = 0;
		r.m_lit.m_wrd = (aclid_get_bit_width(c) + 7) >> 3;
		return r;
	}
}

ae2f_ccpure	aclspv_wrd_t	aclty_getalign(const aclspv_id_t c, const h_aclspv_ctx_t h_ctx) {
	ae2f_ifneznrm_strict(c >= ACLSPV_ID_END) {
		aclty_t* ae2f_restrict cxtp	= aclty_find(c, h_ctx);
		ae2f_ifezerr(cxtp)			return 0;
		return cxtp->m_alignof;
	}

	return (aclid_get_bit_width(c) + 7) >> 3;
}
