#include "./ptr.h"
#include "./implutil.h"

#include <util/iddef.h>
#include <util/emitx.h>

aclspv_id_t aclutil_mk_cxtp_ptr(
		const aclspv_id_t		c_type_elm,
		const enum SpvStorageClass_	c_storage_class,
		const h_aclspv_ctx_t		h_ctx
		)
{
	aclutil_cxtp		TAR;

	memset(&TAR, 0, sizeof(TAR));

	TAR.m_class			= ACLUTIL_CXTYPE_PTR;
	TAR.m_info.m_ptr.m_type		= c_type_elm;
	TAR.m_info.m_ptr.m_storage	= c_storage_class;
	TAR.m_alignof			= h_ctx->m_usrcfg.m_flag0 & ACLSPV_CTX_FLAG0_LOGICAL
		? 0 : h_ctx->m_usrcfg.m_flag0 & ACLSPV_CTX_FLAG0_BUF64 ? 8 : 4;

	TAR.m_sizeof.m_id = 0;
	TAR.m_sizeof.m_lit.m_wrd = TAR.m_alignof;

	(void)(aclid_mk_default_id(c_type_elm, h_ctx));

	{
		const struct __ret ID = s_ret(h_ctx, &TAR); 
		ae2f_expected_but_else(ID.m_id) return 0;
		unless(ID.m_isfirst)		return ID.m_id;

		ae2f_expected_but_else(h_ctx->m_count.m_types = aclutil_emitx_4(
					&h_ctx->m_section.m_types
					, h_ctx->m_count.m_types
					, SpvOpTypePointer
					, ID.m_id
					, (aclspv_wrd_t) c_storage_class
					, c_type_elm
					)) return 0;

		return ID.m_id;
	}
}
