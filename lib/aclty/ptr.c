#include "./ptr.h"
#include "./implutil.h"

#include <util/iddef.h>
#include <util/emitx.h>

#include <aclmem.h>

#include <ae2f/cc/pause.h>

/** 1 when same */
ae2f_inline_f ae2f_ccpure static aclspv_bool_t s_ndeq(
		const aclty_t* const ae2f_restrict A,
		const aclty_t* const ae2f_restrict B
		)
{
	if(B->m_class != ACLTY_PTR) return 0;

	if(B->m_info.m_ptr.m_storage != A->m_info.m_ptr.m_storage)
		return 0;

	if(B->m_info.m_ptr.m_type != A->m_info.m_ptr.m_type)
		return 0;

	return 1;
}

ae2f_inline_f ae2f_ccpure static aclspv_wrdcount_t s_find_node(
		const aclty_t* const ae2f_restrict rd_tar,
		const h_aclspv_ctx_t			h_ctx
		) {
	aclspv_wrdcount_t IDX = (h_ctx)->m_num_type_uniques;
	ae2f_ifezerr(h_ctx->m_type_uniques.m_p)
		return 0xFFFFFFFF;

	ae2f_ifnezerr((h_ctx)->m_num_type_uniques > (aclspv_wrd_t)(h_ctx->m_type_uniques.m_sz / sizeof(aclty_t)))
		return 0xFFFFFFFF;

	while(IDX-- 
			&& !s_ndeq(
				rd_tar
				, ((const aclty_t* ae2f_restrict)h_ctx->m_type_uniques.m_p) + IDX))
	{ ae2fcc_pause(); }

	return IDX;
}

ae2f_inline_f static struct __ret s_ret(const h_aclspv_ctx_t h_ctx, const aclty_t* ae2f_restrict rd_cxtp)
{
	aclspv_wrdcount_t	IDX = s_find_node(rd_cxtp, h_ctx);

	ae2f_ifezerr_strict(IDX == 0xFFFFFFFF) {
		struct	__ret		RET;
		RET.m_id	= __get_type_uniques(h_ctx)[IDX].m_type_id;
		RET.m_isfirst	= 0;
		return RET;
	}

	return s_ret_new(h_ctx, rd_cxtp, 0, 1);
}

aclspv_id_t aclty_mk_ptr(
		const aclspv_id_t		c_type_elm,
		const enum SpvStorageClass_	c_storage_class,
		const h_aclspv_ctx_t		h_ctx
		)
{
	aclty_t		TAR;


	aclmemzero(&TAR, sizeof(TAR));

	TAR.m_class			= ACLTY_PTR;
	TAR.m_info.m_ptr.m_type		= c_type_elm;
	TAR.m_info.m_ptr.m_storage	= c_storage_class;
	TAR.m_alignof			= h_ctx->m_usrcfg.m_flag0 & ACLSPV_CTX_FLAG0_LOGICAL
		? 0 : h_ctx->m_usrcfg.m_flag0 & ACLSPV_CTX_FLAG0_BUF64 ? 8 : 4;

	TAR.m_sizeof.m_id = 0;
	TAR.m_sizeof.m_lit.m_wrd = TAR.m_alignof;

	if(h_ctx->m_usrcfg.m_flag0 & ACLSPV_CTX_FLAG0_LOGICAL) {
		TAR.m_field0 |= ACLTY_FIELD0_NOPOD;
	}

	ae2f_ifezerr(aclid_mk_default_id(c_type_elm, h_ctx))
		return 0;

	{
		const struct __ret ID = s_ret(h_ctx, &TAR); 
		ae2f_ifezerr(ID.m_id) return 0;
		unless(ID.m_isfirst)		return ID.m_id;

		ae2f_ifezerr(h_ctx->m_count.m_types = aclutil_emitx_4(
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
