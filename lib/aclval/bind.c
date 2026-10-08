#include "./bind.h"

aclspv_wrd_t aclval_get_bind_last_index(aclspv_wrd_t c_desc, h_aclspv_ctx_t h) {
	aclval_t* ae2f_restrict v = aclval_cfg_node2(c_desc, 0, h);
	ae2f_ifezerr(v) return 0xFFFFFFFF;
	return v->m_set_bind_last_idx;
}

aclspv_wrd_t aclval_inc_bind_last_index(aclspv_wrd_t c_desc, h_aclspv_ctx_t h) {
	aclval_t* ae2f_restrict v = aclval_cfg_node2(c_desc, 0, h);
	ae2f_ifezerr(v) return 0xFFFFFFFF;
	return v->m_set_bind_last_idx++;
}

aclspv_wrd_t aclval_cfg_bind_storage_class(
		aclspv_wrd_t c_desc,
		aclspv_wrd_t c_bind_idx,
		aclspv_wrd_t c_storage_class,
		h_aclspv_ctx_t h
		) {
	aclval_t* ae2f_restrict v = aclval_cfg_node2(c_desc, c_bind_idx, h);
	ae2f_ifezerr(v) return SpvStorageClassMax;
	return v->m_set_bind_storage_class == SpvStorageClassMax 
		? (v->m_set_bind_storage_class = c_storage_class) 
		: v->m_set_bind_storage_class
		;
}
