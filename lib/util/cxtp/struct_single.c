#include "./struct.h"


#if	0
#include "./implutil.h"
#include <util/emitx.h>
#include <util/iddef.h>
#include <ae2f/cc/memset.h>
#else
#include "../cxtp.h"
#include "./struct/mbr.h"
#include "./struct/dcr.h"
#endif

aclspv_id_t aclutil_mk_cxtp_struct_single(
		const aclspv_id_t	c_type_member,
		const B_aclspv_cxtp_struct_decoration_field0_t
		c_decorate_field0,
		const aclspv_wrd_t	c_memdecorate_offset,
		const h_aclspv_ctx_t	h_ctx
		)
{
	aclspv_id_t id = aclutil_cxtp_struct_cfg(h_ctx);
	aclutil_cxtp* ae2f_restrict cxtp = aclutil_find_cxtp(id, h_ctx);
	ae2f_ifezerr(cxtp) {
		return 0;
	}

	if(c_memdecorate_offset ^ 0xFFFFFFFF) {
		aclutil_cxtp* ae2f_restrict mbr = aclutil_cxtp_struct_add_mbr_un(cxtp, c_type_member, h_ctx);
		ae2f_ifezerr(mbr)	return 0;
	}

	if(c_decorate_field0 & ACLSPV_CXTP_STRUCT_DECORATION_FIELD0_BLOCK) {
		aclutil_cxtp* ae2f_restrict dcr = aclutil_cxtp_struct_dcr_cfg(cxtp, SpvDecorationBlock, h_ctx);
		dcr->m_info.m_dcr.m_num_oprnd = 0;
	}

	{
		aclutil_cxtp* ae2f_restrict ret = aclutil_cxtp_struct_build(id, h_ctx);
		ae2f_ifezerr(ret) return 0;
		return ret->m_type_id;
	}
}
