#ifndef	implutil_structdcr_h
#define implutil_structdcr_h

#include "./implutil.h"

static aclutil_cxtp* s_get_insort_dir_dcr(aclutil_cxtp* ae2f_restrict rdwr_dcr, aclspv_wrd_t c_dcr, h_aclspv_ctx_t h_ctx) {
	do {
		if(rdwr_dcr->m_info.m_struct_decor.m_op > (aclspv_wrd_t)c_dcr)
			return rdwr_dcr;

		rdwr_dcr = rdwr_dcr->m_info.m_struct_decor.m_nxt
			? &__get_type_uniques(h_ctx)[(rdwr_dcr->m_info.m_struct_decor.m_nxt)]
			: 0;
	} while(rdwr_dcr);

	return 0;
}

ae2f_ccpure static aclutil_cxtp* s_get_last_decor(aclutil_cxtp* ae2f_restrict rd_dcr, h_aclspv_ctx_t h_ctx) {
	while(rd_dcr->m_info.m_struct_decor.m_nxt)
	{
		rd_dcr = &__get_type_uniques(h_ctx)[(rd_dcr->m_info.m_struct_decor.m_nxt)];
	}

	return rd_dcr;
}

static aclutil_cxtp* s_dcr_insert_invoke(
		aclspv_wrd_t c_dcr,
		aclutil_cxtp* const ae2f_restrict rdwr_dcr_new,
		aclutil_cxtp* ae2f_restrict rdwr_dcr_root,
		h_aclspv_ctx_t h_ctx
		) {
	aclutil_cxtp
		* ae2f_restrict inc = s_get_insort_dir_dcr(rdwr_dcr_root, c_dcr, h_ctx); /***/

	if(inc) {
		aclutil_cxtp
			* ae2f_restrict Prv = &__get_type_uniques(h_ctx)[inc->m_info.m_struct_decor.m_prv],
			* ae2f_restrict Nxt = &__get_type_uniques(h_ctx)[inc->m_info.m_struct_decor.m_nxt]; /**/

		rdwr_dcr_new->m_info.m_struct_decor.m_nxt = (aclspv_wrd_t)(Nxt - __get_type_uniques(h_ctx));
		rdwr_dcr_new->m_info.m_struct_decor.m_prv = (aclspv_wrd_t)(Prv - __get_type_uniques(h_ctx));

		Prv->m_info.m_struct_decor.m_nxt = (aclspv_wrd_t)(rdwr_dcr_new - __get_type_uniques(h_ctx));
		rdwr_dcr_new->m_info.m_struct_decor.m_prv = (aclspv_wrd_t)(rdwr_dcr_new - __get_type_uniques(h_ctx));
	} else {
		aclutil_cxtp* ae2f_restrict Lst = s_get_last_decor(rdwr_dcr_root, h_ctx);

		Lst->m_info.m_struct_decor.m_nxt = (aclspv_wrd_t)(rdwr_dcr_new - __get_type_uniques(h_ctx));
		rdwr_dcr_new->m_info.m_struct_decor.m_prv = (aclspv_wrd_t)(Lst - __get_type_uniques(h_ctx));
	}

	return rdwr_dcr_new;
}

#endif
