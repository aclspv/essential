#include "../aclty.h"
#include "./struct.h"
#include "./struct/mbr.h"
#include "./struct/dcr.h"

aclspv_id_t aclty_mk_struct_single(
		const aclspv_id_t	c_type_member,
		const B_aclspv_cxtp_struct_decoration_field0_t
		c_decorate_field0,
		const aclspv_wrd_t	c_memdecorate_offset,
		const h_aclspv_ctx_t	h_ctx
		)
{
	aclspv_id_t id = aclty_cfg_struct(h_ctx);
	aclutil_cxtp* ae2f_restrict cxtp = aclty_find(id, h_ctx);
	ae2f_ifezerr(cxtp) {
		return 0;
	}

	if(c_memdecorate_offset ^ 0xFFFFFFFF) {
		aclutil_cxtp* ae2f_restrict mbr = aclty_add_struct_mbr_un(cxtp, c_type_member, h_ctx);
		ae2f_ifezerr(mbr)	return 0;
	}

	if(c_decorate_field0 & ACLSPV_CXTP_STRUCT_DECORATION_FIELD0_BLOCK) {
		aclutil_cxtp* ae2f_restrict dcr = aclty_cfg_struct_dcr(cxtp, SpvDecorationBlock, h_ctx);
		dcr->m_info.m_dcr.m_num_oprnd = 0;
	}

	{
		aclutil_cxtp* ae2f_restrict ret = aclty_build_struct(id, h_ctx);
		ae2f_ifezerr(ret) return 0;
		return ret->m_type_id;
	}
}
