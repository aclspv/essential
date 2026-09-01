#ifndef	aclutil_cxtp_struct_h

#include "./common.h"

aclspv_id_t aclty_mk_struct_single(
		const aclspv_id_t	c_type_member,
		const B_aclspv_cxtp_struct_decoration_field0_t	
		c_decorate_field0,
		const aclspv_wrd_t	c_memdecorate_offset,
		const h_aclspv_ctx_t	h_ctx
		);

/** STRUCT DECORATION */

aclspv_id_t aclty_cfg_struct(h_aclspv_ctx_t h_ctx);
aclspv_id_t aclty_cfg_struct2(h_aclspv_ctx_t h_ctx, aclspv_id_t c);
aclutil_cxtp* aclty_build_struct(aclspv_id_t c_id, h_aclspv_ctx_t h_ctx);

#include "./struct/mbr.h"
#include "./struct/dcr.h"

#endif
