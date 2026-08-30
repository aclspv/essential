#ifndef aclcxtp_struct_mbr_h
#define aclcxtp_struct_mbr_h

#include "../struct.h"

aclutil_cxtp* aclutil_cxtp_struct_add_mbr(
		aclutil_cxtp* ae2f_restrict rdwr_struct,
		aclspv_id_t c_id,
		h_aclspv_ctx_t h_ctx);

typedef aclutil_cxtp* aclutil_cxtp_struct_add_mbr_t(
		aclutil_cxtp* ae2f_restrict rdwr_struct,
		aclspv_id_t c_id,
		h_aclspv_ctx_t h_ctx);


aclutil_cxtp_struct_add_mbr_t aclutil_cxtp_struct_add_mbr_grow,
			      aclutil_cxtp_struct_add_mbr_un;

aclutil_cxtp* aclutil_cxtp_struct_add_mbr_grow(
		aclutil_cxtp* ae2f_restrict rdwr_struct,
		aclspv_id_t c_id,
		h_aclspv_ctx_t h_ctx);

/** non-grow (union) */
aclutil_cxtp* aclutil_cxtp_struct_add_mbr_un(
		aclutil_cxtp* ae2f_restrict rdwr_struct,
		aclspv_id_t c_id,
		h_aclspv_ctx_t h_ctx);


ae2f_ccpure aclutil_cxtp* aclutil_cxtp_struct_get_mbr(
		aclutil_cxtp* ae2f_restrict rdwr_struct,
		aclspv_wrd_t c_idx,
		h_aclspv_ctx_t h_ctx);

aclutil_cxtp* aclutil_cxtp_struct_mbr_dcr_cfg(aclutil_cxtp* ae2f_restrict rdwr_mbr, SpvDecoration c_decor, h_aclspv_ctx_t h_ctx);
aclutil_cxtp* aclutil_cxtp_struct_mbr_dcr_cfg2(aclspv_id_t c_id, SpvDecoration c_decor, h_aclspv_ctx_t h_ctx);
void aclutil_cxtp_free_struct_mbr(aclutil_cxtp* ae2f_restrict fini, h_aclspv_ctx_t h_ctx);

#endif
