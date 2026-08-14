#ifndef aclutil_cxtp_struct_dcr_h
#define aclutil_cxtp_struct_dcr_h

#include <util/cxtp.h>

/** STRUCT DECORATION */
aclutil_cxtp* aclutil_cxtp_struct_dcr_cfg(aclutil_cxtp* ae2f_restrict rdwr_struct, SpvDecoration c_decor, h_aclspv_ctx_t h_ctx);
aclutil_cxtp* aclutil_cxtp_struct_dcr_cfg2(aclspv_id_t c_id, SpvDecoration c_decor, h_aclspv_ctx_t h_ctx);

#endif
