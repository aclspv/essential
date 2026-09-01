#ifndef	aclutil_cxtp_dcr_h
#define aclutil_cxtp_dcr_h

#include "../aclty.h"

ae2f_ccpure aclutil_cxtp* aclty_get_dcr(
		aclutil_cxtp* ae2f_restrict rd_dcr,
		aclspv_wrd_t c_decor,
		h_aclspv_ctx_t h_ctx
		);
ae2f_ccpure aclspv_bool_t aclty_is_dcr_same(
		aclutil_cxtp* ae2f_restrict rd0,
		aclutil_cxtp* ae2f_restrict rd1,
		h_aclspv_ctx_t		h_ctx
		);

void aclty_free_dcr(aclutil_cxtp* ae2f_restrict fini, h_aclspv_ctx_t h_ctx);

/** 0 on success */
aclspv_bool_t aclty_build_dcr(const aclutil_cxtp* ae2f_restrict rd, aclspv_id_t c_id, h_aclspv_ctx_t h_ctx);
aclspv_bool_t aclty_build_dcr_mdcr(const aclutil_cxtp* ae2f_restrict rd, aclspv_id_t c_id, aclspv_wrd_t c_idx, h_aclspv_ctx_t h_ctx);

#endif
