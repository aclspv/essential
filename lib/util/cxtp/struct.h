#ifndef	aclutil_cxtp_struct_h

#include "./common.h"

aclspv_id_t aclutil_mk_cxtp_struct_single(
		const aclspv_id_t	c_type_member,
		const B_aclspv_cxtp_struct_decoration_field0_t	
		c_decorate_field0,
		const aclspv_wrd_t	c_memdecorate_offset,
		const h_aclspv_ctx_t	h_ctx
		);

/** STRUCT DECORATION */
aclutil_cxtp* aclutil_cxtp_struct_dcr_cfg(aclutil_cxtp* ae2f_restrict rdwr_struct, SpvDecoration c_decor, h_aclspv_ctx_t h_ctx);
aclutil_cxtp* aclutil_cxtp_struct_dcr_cfg2(aclspv_id_t c_id, SpvDecoration c_decor, h_aclspv_ctx_t h_ctx);
aclutil_cxtp* aclutil_cxtp_get_dcr(aclutil_cxtp* ae2f_restrict rd_dcr, aclspv_wrd_t c_decor, h_aclspv_ctx_t h_ctx);



#endif
