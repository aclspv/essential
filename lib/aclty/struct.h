#ifndef	aclutil_cxtp_struct_h
#define aclutil_cxtp_struct_h

#include "./common.h"

enum ACLTY_STRUCT_FIELD0 {
	ACLTY_STRUCT_FIELD0_BLOCK = 1,
	/** the record derives from a C union: its C90 initializer is a
	 * single element for the first member; the rest are null. */
	ACLTY_STRUCT_FIELD0_UNION = 2
};

/** @see enum ACLTY_STRUCT_FIELD0 */
typedef aclspv_wrd_t aclty_struct_field0_t;

aclspv_id_t aclty_mk_struct_single_blk(
		const aclspv_id_t	c_type_member,
		const aclty_struct_field0_t	
		c_decorate_field0,
		const aclspv_wrd_t	c_memdecorate_offset,
		const h_aclspv_ctx_t	h_ctx
		);

/** STRUCT DECORATION */

aclspv_id_t aclty_cfg_struct(h_aclspv_ctx_t h_ctx);
aclspv_id_t aclty_cfg_struct2(h_aclspv_ctx_t h_ctx, aclspv_id_t c);
aclty_t* aclty_build_struct(aclspv_id_t c_id, h_aclspv_ctx_t h_ctx);

#include "./struct/mbr.h"
#include "./struct/dcr.h"

#endif
