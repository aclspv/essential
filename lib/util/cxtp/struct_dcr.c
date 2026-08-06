#include "./struct.h"

#include "./implutil.h"
#include "./implutil_structdcr.h"

#include <ae2f/cc/memset.h>
#include <ae2f/pat/dbg.h>
#include <util/cxtp.h>

/*** STRUCT DECORATION ***/

aclutil_cxtp* aclutil_cxtp_struct_dcr_cfg2(aclspv_id_t c_id, SpvDecoration c_decor, h_aclspv_ctx_t h_ctx) {
	aclutil_cxtp* ae2f_restrict cxtp = aclutil_find_cxtp(c_id, h_ctx);
	return ae2f_expected(cxtp) ? aclutil_cxtp_struct_dcr_cfg(cxtp, c_decor, h_ctx) : 0;
}

ae2f_ccpure aclutil_cxtp* aclutil_cxtp_get_dcr(aclutil_cxtp* ae2f_restrict rd_dcr, aclspv_wrd_t c_decor, h_aclspv_ctx_t h_ctx) {
	ae2fpat_assertassumeunless(rd_dcr) {
		h_ctx->m_err = ACLSPV_COMPILE_CXTP_ZERO;
		return 0;
	}

	do {
		if(rd_dcr->m_info.m_struct_decor.m_op == (aclspv_wrd_t)c_decor)
			return rd_dcr;

		rd_dcr = rd_dcr->m_info.m_struct_decor.m_nxt
			? &__get_type_uniques(h_ctx)[(rd_dcr->m_info.m_struct_decor.m_nxt)]
			: 0;
	} while(rd_dcr);

	return 0;
}

aclutil_cxtp* aclutil_cxtp_struct_dcr_cfg(aclutil_cxtp* ae2f_restrict rdwr_struct, SpvDecoration c_decor, h_aclspv_ctx_t h_ctx) {
	aclutil_cxtp*	r = 0;

	ae2fpat_assertassumeunless(rdwr_struct) {
		h_ctx->m_err = ACLSPV_COMPILE_CXTP_ZERO;
		return 0;
	}

	ae2fpat_assertassumeunless(rdwr_struct->m_class == ACLUTIL_CXTYPE_STRUCT) {
		h_ctx->m_err = ACLSPV_COMPILE_CXTP_TYINVL_STRUCT_DCR;
		return 0;
	}

	if(rdwr_struct->m_info.m_struct.m_dcr) {
		aclutil_cxtp* g;
		if((g = aclutil_cxtp_get_dcr(
						&__get_type_uniques(h_ctx)[rdwr_struct->m_info.m_struct.m_dcr]
						, c_decor
						, h_ctx)))
			return g;
	}

	{
		struct __ret ID;
		aclutil_cxtp TAR;
		ae2fcc_memset(&TAR, 0, sizeof(TAR));

		TAR.m_class	= ACLUTIL_CXTYPE_STRUCT_DCR;
		TAR.m_info.m_struct_decor.m_op		= (aclspv_wrd_t)c_decor;
		TAR.m_info.m_struct_decor.m_nxt		= 0;

		ID = s_ret_new(h_ctx, &TAR, h_ctx->m_id++);
		unless(ID.m_isfirst)	return 0;

		r = aclutil_find_cxtp(ID.m_id, h_ctx);
	}

	ae2f_ifezerr(r)	return 0;

	++rdwr_struct->m_info.m_struct.m_num_dcr;
	unless(rdwr_struct->m_info.m_struct.m_dcr) {
		rdwr_struct->m_info.m_struct.m_dcr = (aclspv_wrd_t)(r - __get_type_uniques(h_ctx));
		return r;
	}

	return s_dcr_insert_invoke(
			(aclspv_wrd_t)c_decor
			, r
			, &(__get_type_uniques(h_ctx)[rdwr_struct->m_info.m_struct.m_dcr])
			, h_ctx
			);
}
