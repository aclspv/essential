#ifndef aclutil_cxtp_struct_dcr_h
#define aclutil_cxtp_struct_dcr_h

#include "../common.h"

/** STRUCT DECORATION */
aclty_t* aclty_cfg_struct_dcr(aclty_t* ae2f_restrict rdwr_struct, SpvDecoration c_decor, h_aclspv_ctx_t h_ctx);
aclty_t* aclty_cfg_struct_dcr1(aclspv_id_t c_id, SpvDecoration c_decor, h_aclspv_ctx_t h_ctx);

#endif
