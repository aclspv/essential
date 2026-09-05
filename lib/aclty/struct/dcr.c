#include "./dcr.h"
#include "../dcr.h"
#include "../dcr/util.h"

#include <aclmem.h>

#include <ae2f/pat/dbg.h>

/*** STRUCT DECORATION ***/

aclty_t* aclty_cfg_struct_dcr1(aclspv_id_t c_id, SpvDecoration c_decor, h_aclspv_ctx_t h_ctx) {
	aclty_t* ae2f_restrict cxtp = aclty_find(c_id, h_ctx);
	return ae2f_expected(cxtp) ? aclty_cfg_struct_dcr(cxtp, c_decor, h_ctx) : 0;
}

aclty_t* aclty_cfg_struct_dcr(aclty_t* ae2f_restrict rdwr_struct, SpvDecoration c_decor, h_aclspv_ctx_t h_ctx) {
	aclty_t*	r = 0;

	ae2fpat_assertassumeunless(rdwr_struct) {
		h_ctx->m_err = ACLSPV_COMPILE_CXTP_ZERO;
		return 0;
	}

	ae2fpat_assertassumeunless(rdwr_struct->m_class == ACLTY_STRUCT) {
		h_ctx->m_err = ACLSPV_COMPILE_CXTP_TYINVL_STRUCT_DCR;
		return 0;
	}

	if(rdwr_struct->m_info.m_struct.m_dcr) {
		aclty_t* g;
		if((g = aclty_get_dcr(
						&__get_type_uniques(h_ctx)[rdwr_struct->m_info.m_struct.m_dcr]
						, c_decor
						, h_ctx)))
			return g;
	}

	{
		struct __ret	ID;
		aclty_t	TAR;
		ptrdiff_t	pad = rdwr_struct - __get_type_uniques(h_ctx);

		aclmemzero(&TAR, sizeof(TAR));

		TAR.m_class	= ACLTY_DCR;
		TAR.m_info.m_dcr.m_op		= (aclspv_wrd_t)c_decor;
		TAR.m_info.m_dcr.m_nxt		= 0;

		ID = s_ret_new(h_ctx, &TAR, h_ctx->m_id++, 0);
		rdwr_struct = &__get_type_uniques(h_ctx)[pad];

		unless(ID.m_id)	return 0;

		r = aclty_find(ID.m_id, h_ctx);
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
