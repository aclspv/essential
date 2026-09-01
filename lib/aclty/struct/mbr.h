#ifndef aclcxtp_struct_mbr_h
#define aclcxtp_struct_mbr_h

#include "../struct.h"

aclutil_cxtp* aclutil_cxtp_struct_add_mbr(
		aclutil_cxtp* ae2f_restrict rdwr_struct,
		aclspv_id_t c_id,
		h_aclspv_ctx_t h_ctx);

typedef aclutil_cxtp* aclty_add_struct_mbr_t(
		aclutil_cxtp* ae2f_restrict rdwr_struct,
		aclspv_id_t c_id,
		h_aclspv_ctx_t h_ctx);


aclty_add_struct_mbr_t		aclty_add_struct_mbr_std430,
				aclty_add_struct_mbr_std140,
				aclty_add_struct_mbr_un;

aclutil_cxtp* aclty_add_struct_mbr_std430(
		aclutil_cxtp* ae2f_restrict rdwr_struct,
		aclspv_id_t c_id,
		h_aclspv_ctx_t h_ctx);

/** TODO */
aclutil_cxtp* aclty_add_struct_mbr_std140(
		aclutil_cxtp* ae2f_restrict rdwr_struct,
		aclspv_id_t c_id,
		h_aclspv_ctx_t h_ctx);

/** non-grow (union) */
aclutil_cxtp* aclty_add_struct_mbr_un(
		aclutil_cxtp* ae2f_restrict rdwr_struct,
		aclspv_id_t c_id,
		h_aclspv_ctx_t h_ctx);


ae2f_ccpure aclutil_cxtp* aclty_get_struct_mbr(
		aclutil_cxtp* ae2f_restrict rdwr_struct,
		aclspv_wrd_t c_idx,
		h_aclspv_ctx_t h_ctx);

aclutil_cxtp* aclty_cfg_struct_mdcr(aclutil_cxtp* ae2f_restrict rdwr_mbr, SpvDecoration c_decor, h_aclspv_ctx_t h_ctx);
aclutil_cxtp* aclty_cfg_struct_mdcr1(aclspv_id_t c_id, SpvDecoration c_decor, h_aclspv_ctx_t h_ctx);
void aclty_free_struct_mbr(aclutil_cxtp* ae2f_restrict fini, h_aclspv_ctx_t h_ctx);

#endif
