#ifndef aclutil_cxtp_struct_dcr_h
#define aclutil_cxtp_struct_dcr_h

#include "../common.h"

/** STRUCT DECORATION */
aclutil_cxtp* aclty_cfg_struct_dcr(aclutil_cxtp* ae2f_restrict rdwr_struct, SpvDecoration c_decor, h_aclspv_ctx_t h_ctx);
aclutil_cxtp* aclty_cfg_struct_dcr1(aclspv_id_t c_id, SpvDecoration c_decor, h_aclspv_ctx_t h_ctx);

#endif
