#include "./struct.h"
#include "./implutil.h"

#include <util/emitx.h>
#include <util/iddef.h>
#include <ae2f/cc/memset.h>

aclspv_id_t aclutil_mk_cxtp_struct_single(
		const aclspv_id_t	c_type_member,
		const B_aclspv_cxtp_struct_decoration_field0_t
		c_decorate_field0,
		const aclspv_wrd_t	c_memdecorate_offset,
		const h_aclspv_ctx_t	h_ctx
		)
{
	aclutil_cxtp		TAR;

	ae2fcc_memset(&TAR, 0, sizeof(TAR));

	TAR.m_class						= ACLUTIL_CXTYPE_STRUCT_SINGLE;
	TAR.m_info.m_struct_single.m_mem_id			= c_type_member;
	TAR.m_info.m_struct_single.m_decoration.m_field0	= c_decorate_field0;
	TAR.m_info.m_struct_single.m_mem_decoration.m_offset	= c_memdecorate_offset;

	(void)(aclid_mk_default_id(c_type_member, h_ctx));

	{
		const struct __ret ID = s_ret(h_ctx, &TAR);
		ae2f_expected_but_else(ID.m_id) return 0;
		unless(ID.m_isfirst)		return ID.m_id;

		ae2f_expected_but_else(h_ctx->m_count.m_types = util_emitx_3(
					&h_ctx->m_section.m_types
					, h_ctx->m_count.m_types
					, SpvOpTypeStruct
					, ID.m_id
					, c_type_member
					)) return 0;

		if(c_decorate_field0 & (ACLSPV_CXTP_STRUCT_DECORATION_FIELD0_BLOCK))
			ae2f_expected_but_else(h_ctx->m_count.m_decorate = util_emitx_3(
						&h_ctx->m_section.m_decorate
						, h_ctx->m_count.m_decorate
						, SpvOpDecorate
						, ID.m_id
						, SpvDecorationBlock
						)) return 0;

		if(c_memdecorate_offset ^ 0xFFFFFFFF) {
			ae2f_expected_but_else(h_ctx->m_count.m_decorate = util_emitx_5(
						&h_ctx->m_section.m_decorate
						, h_ctx->m_count.m_decorate
						, SpvOpMemberDecorate
						, ID.m_id
						, 0
						, SpvDecorationOffset
						, 0
						)) return 0;
		}

		return ID.m_id;
	}
}
