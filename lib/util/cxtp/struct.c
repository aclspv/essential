#include "./struct.h"

aclspv_id_t aclutil_cxtp_struct_cfg(h_aclspv_ctx_t h_ctx);

aclutil_cxtp* aclutil_cxtp_struct_add_mbr(
		aclutil_cxtp* ae2f_restrict rdwr_struct,
		aclspv_id_t c_id,
		h_aclspv_ctx_t h_ctx);

aclutil_cxtp* aclutil_cxtp_struct_get_mbr(
		aclutil_cxtp* ae2f_restrict rdwr_struct,
		aclspv_wrd_t c_idx,
		h_aclspv_ctx_t h_ctx);

/*******************************************************/
/*******************************************************/

#include "./implutil.h"

#include <util/cxtp.h>
#include <ae2f/cc/memset.h>
#include <ae2f/pat/dbg.h>

/*** STRUCT MEMBER ***/

ae2f_ccpure static aclutil_cxtp* s_get_last_mbr(aclutil_cxtp* ae2f_restrict rd_mbr, h_aclspv_ctx_t h_ctx) {
	while(rd_mbr->m_info.m_struct_decor.m_nxt)
	{
		rd_mbr = &__get_type_uniques(h_ctx)[(rd_mbr->m_info.m_struct_mbr.m_nxt)];
	}

	return rd_mbr;
}

aclutil_cxtp* aclutil_cxtp_struct_get_mbr(
		aclutil_cxtp* ae2f_restrict rdwr_struct,
		aclspv_wrd_t c_idx,
		h_aclspv_ctx_t h_ctx) {
	aclutil_cxtp* ae2f_restrict R;

	ae2f_ifnezerr_strict(rdwr_struct->m_info.m_struct.m_num_mbr <= c_idx)
		return 0;

	R = &__get_type_uniques(h_ctx)[rdwr_struct->m_info.m_struct.m_mbr];

	while(R->m_info.m_struct_mbr.m_nxt)
	{
		if(R->m_info.m_struct_mbr.m_idx == c_idx)
			return R;

		R = &__get_type_uniques(h_ctx)[(R->m_info.m_struct_mbr.m_nxt)];
	}

	/** UNLIKELY */
	ae2f_ifnezerr_strict(1) {
		h_ctx->m_err = ACLSPV_COMPILE_CXTP_STRUCT_MBR_NUM_IS_LYING;
		return 0;
	} else {
		ae2f_unreachable();
	}
}

aclutil_cxtp* aclutil_cxtp_struct_add_mbr(aclutil_cxtp* ae2f_restrict rdwr_struct, aclspv_id_t c_id, h_aclspv_ctx_t h_ctx) {
	aclutil_cxtp* ae2f_restrict r, * ae2f_restrict r2;

	ae2fpat_assertassumeunless(rdwr_struct) {
		h_ctx->m_err = ACLSPV_COMPILE_CXTP_ZERO;
		return 0;
	}

	ae2fpat_assertassumeunless(rdwr_struct->m_class == ACLUTIL_CXTYPE_STRUCT) {
		h_ctx->m_err = ACLSPV_COMPILE_CXTP_TYINVL_STRUCT_DCR;
		return 0;
	}

	{
		struct __ret ID;
		aclutil_cxtp TAR;
		ae2fcc_memset(&TAR, 0, sizeof(TAR));

		TAR.m_class				= ACLUTIL_CXTYPE_STRUCT_MBR;
		TAR.m_info.m_struct_mbr.m_id_mbr	= c_id;
		TAR.m_info.m_struct_mbr.m_idx		= rdwr_struct->m_info.m_struct.m_num_mbr++;

		ID = s_ret_new(h_ctx, &TAR, h_ctx->m_id++);
		ae2f_ifezerr(ID.m_isfirst) {
			--rdwr_struct->m_info.m_struct.m_num_mbr;
			return 0;
		}

		r = aclutil_find_cxtp(ID.m_id, h_ctx);
	}

	if(rdwr_struct->m_info.m_struct.m_mbr) {
		rdwr_struct->m_info.m_struct.m_mbr = (aclspv_wrd_t)(r - __get_type_uniques(h_ctx));
		return r;
	}

	r2 = s_get_last_mbr(&__get_type_uniques(h_ctx)[rdwr_struct->m_info.m_struct.m_mbr], h_ctx);
	r2->m_info.m_struct_mbr.m_nxt = (aclspv_wrd_t)(r - __get_type_uniques(h_ctx));

	ae2f_ifnezerr_strict(r2->m_info.m_struct_mbr.m_idx + 1 != r->m_info.m_struct_mbr.m_idx) {
		h_ctx->m_err = ACLSPV_COMPILE_CXTP_STRUCT_MBR_NUM_IS_LYING;
		return 0;
	}

	return r;
}

/*** STRUCT CONFIGURATION ***/
aclspv_id_t aclutil_cxtp_struct_cfg(h_aclspv_ctx_t h_ctx) {
	aclutil_cxtp TAR;
	ae2fcc_memset(&TAR, 0, sizeof(TAR));

	TAR.m_class	= ACLUTIL_CXTYPE_STRUCT;

	{
		const struct __ret ID = s_ret_new(h_ctx, &TAR, h_ctx->m_id++);
		ae2f_expected_but_else(ID.m_id) return 0;
		return	ID.m_id;
	}
}
