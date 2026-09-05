#ifndef aclcxtp_struct_mbr_h
#define aclcxtp_struct_mbr_h

#include "../struct.h"

aclty_t* aclty_struct_add_mbr(
		aclty_t* ae2f_restrict rdwr_struct,
		aclspv_id_t c_id,
		h_aclspv_ctx_t h_ctx);

typedef aclty_t* aclty_add_struct_mbr_t(
		aclty_t* ae2f_restrict rdwr_struct,
		aclspv_id_t c_id,
		h_aclspv_ctx_t h_ctx);


aclty_add_struct_mbr_t		aclty_add_struct_mbr_std430,
				aclty_add_struct_mbr_std140,
				aclty_add_struct_mbr_un;

aclty_t* aclty_add_struct_mbr_std430(
		aclty_t* ae2f_restrict rdwr_struct,
		aclspv_id_t c_id,
		h_aclspv_ctx_t h_ctx);

/** TODO */
aclty_t* aclty_add_struct_mbr_std140(
		aclty_t* ae2f_restrict rdwr_struct,
		aclspv_id_t c_id,
		h_aclspv_ctx_t h_ctx);

/** non-grow (union) */
aclty_t* aclty_add_struct_mbr_un(
		aclty_t* ae2f_restrict rdwr_struct,
		aclspv_id_t c_id,
		h_aclspv_ctx_t h_ctx);


ae2f_ccpure aclty_t* aclty_get_struct_mbr(
		aclty_t* ae2f_restrict rdwr_struct,
		aclspv_wrd_t c_idx,
		h_aclspv_ctx_t h_ctx);

aclty_t* aclty_cfg_struct_mdcr(aclty_t* ae2f_restrict rdwr_mbr, SpvDecoration c_decor, h_aclspv_ctx_t h_ctx);
aclty_t* aclty_cfg_struct_mdcr1(aclspv_id_t c_id, SpvDecoration c_decor, h_aclspv_ctx_t h_ctx);
void aclty_free_struct_mbr(aclty_t* ae2f_restrict fini, h_aclspv_ctx_t h_ctx);

#endif
