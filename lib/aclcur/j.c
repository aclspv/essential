#include "./j.h"
#include "./common.h"

aclspv_id_t	aclcur_cfg_jmp(
		aclspv_id_t c_lbl,
		h_aclspv_ctx_t h_ctx
		) {
	aclspv_id_t	id_jmp = aclcur_cfg_node(0, h_ctx);
	h_aclcur_t	p_lbl = aclcur_get_node(c_lbl, h_ctx);
	h_aclcur_t	p_jmp = aclcur_get_node(id_jmp, h_ctx);

	ae2f_ifezerr(c_lbl)	return 0;
	ae2f_ifezerr(p_jmp)	return 0;

	p_jmp->m_class		= ACLCUR_J;
	p_jmp->m_d.m_j.m_lbl	= c_lbl;

	unless(p_lbl) return id_jmp;
	ae2f_ifezerr(p_lbl->m_class == ACLCUR_LBL)
		return 0;

	(void)++p_lbl->m_num_ref;
	return id_jmp;
}
