#include "./nil.h"
#include <aclval.h>
#include <aclutil/emitx.h>
#include <aclctx/id.h>

aclspv_id_t aclval_cfg_nil(aclspv_id_t c_ty, h_aclspv_ctx_t h) {
	aclval_t* ae2f_restrict v = aclval_cfg_node2(0, c_ty, h);
	ae2f_ifezerr(v) return 0;
	if(v->m_nil)	return v->m_nil;
	else ae2f_ifezerr(h->m_count.m_types = aclutil_emitx_3(
				&h->m_section.m_types
				, h->m_count.m_types
				, SpvOpConstantNull
				, c_ty
				, v->m_nil = aclctx_newid(h)
				)) return 0;

	return v->m_nil;
}
