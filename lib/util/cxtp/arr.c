#include "./arr.h"
#include "./implutil.h"

#include <util/emitx.h>
#include <util/iddef.h>
#include <ae2f/cc/memset.h>

aclspv_id_t aclutil_mk_cxtp_arr2(
		const aclspv_id_t	c_type_elm,
		const aclspv_id_t	c_num_arr,
		const aclspv_wrd_t	c_decorate_stride,
		const h_aclspv_ctx_t	h_ctx
		) {
	aclutil_cxtp            TAR;

	ae2fcc_memset(&TAR, 0, sizeof(TAR));

	TAR.m_class = ACLUTIL_CXTYPE_ARRAY;
	TAR.m_info.m_arr.m_num_id		= c_num_arr;
	TAR.m_info.m_arr.m_type			= c_type_elm;
	TAR.m_info.m_arr.m_decorate_stride	= c_decorate_stride;

	(void)(aclid_mk_default_id(c_type_elm, h_ctx));

	{
		const struct __ret ID = s_ret(h_ctx, &TAR);
		ae2f_expected_but_else(ID.m_id) return 0;
		unless(ID.m_isfirst)            return ID.m_id;

		ae2f_expected_but_else(h_ctx->m_count.m_types = util_emitx_4(
					&h_ctx->m_section.m_types
					, h_ctx->m_count.m_types
					, SpvOpTypeArray
					, ID.m_id
					, c_type_elm
					, c_num_arr
					)) return 0;

		if(c_decorate_stride ^ 0xFFFFFFFF)
			ae2f_expected_but_else(h_ctx->m_count.m_decorate = util_emitx_4(
						&h_ctx->m_section.m_decorate
						, h_ctx->m_count.m_decorate
						, SpvOpDecorate
						, ID.m_id
						, SpvDecorationArrayStride
						, c_decorate_stride
						)) return 0;

		return ID.m_id;
	}
}
