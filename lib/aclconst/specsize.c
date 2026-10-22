
#include "./specsize.h"
#include <aclval.h>
#include <util/iddef.h>

aclspv_bool_t aclconst_is_specsz_same(aclconst_specsz c_x, aclconst_specsz c_y, h_aclspv_ctx_t h_ctx) {
	if(c_x.m_id && c_y.m_id) {
		return c_x.m_id == c_y.m_id;
	}

	if(!c_x.m_id && !c_y.m_id) {
		return c_x.m_lit.m_max == c_y.m_lit.m_max;
	}

	{
		aclspv_id_t	szty = aclid_getty_uint(h_ctx->m_usrcfg.m_byte_size_type << 3);
		ae2f_ifezerr(aclid_mk_default_id(szty, h_ctx)) {
			h_ctx->m_err = ACLSPV_COMPILE_ALLOC_FAILED_ID;
			return 0;
		}

		unless(c_x.m_id) c_x.m_id = aclval_cfg_by_enum(
				c_x.m_lit.m_max, h_ctx, (enum ACLSPV_ID_)szty);

		unless(c_y.m_id) c_y.m_id = aclval_cfg_by_enum(
				c_y.m_lit.m_max, h_ctx, (enum ACLSPV_ID_)szty);

		ae2f_ifnezerr_strict(!c_x.m_id && !c_y.m_id) {
			h_ctx->m_err = ACLSPV_COMPILE_ALLOC_FAILED_ID;
			return 0;
		}

		return c_x.m_id == c_y.m_id;
	}
}
