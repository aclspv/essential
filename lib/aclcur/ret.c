#include "./ret.h"
#include "./common.h"
#include "./util.h"

aclspv_id_t aclcur_cfg_ret(
		aclspv_id_t c_val_opt,
		h_aclspv_ctx_t h_ctx
		) {
	h_aclcur_t p_ret;
	aclspv_id_t	id_ret;

	ae2f_ifezerr(id_ret = aclcur_cfg_node(
				0, h_ctx
				))
		return 0;

	ae2f_ifezerr(p_ret = aclcur_get_node(id_ret, h_ctx))
		return 0;

	p_ret->m_class	= ACLCUR_RET;
	p_ret->m_val	= c_val_opt;

	return id_ret;
}
