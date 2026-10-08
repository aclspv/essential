#include "./jc.h"
#include "./common.h"
#include "./load/get.h"

aclspv_id_t	aclcur_cfg_jc(
		aclspv_id_t	c_cond,
		aclspv_id_t	c_lbl_true,
		aclspv_id_t	c_lbl_false,
		aclspv_wrd_t	c_brw_true,
		aclspv_wrd_t	c_brw_false,
		aclspv_id_t	c_merge_opt,
		aclspv_id_t	c_continue_opt,
		aclspv_wrd_t	c_ctl,
		h_aclspv_ctx_t	h_ctx
		) {
	aclspv_id_t	id;
	h_aclcur_t	p;
	h_aclcur_t p_lbl_true	= aclcur_get_node(c_lbl_true, h_ctx);
	h_aclcur_t p_lbl_false	= aclcur_get_node(c_lbl_false, h_ctx);
	aclcur_t	b_cond	= aclcur_get_load(c_cond, h_ctx);
	h_aclcur_t p_cond	= b_cond.m_id ? aclcur_get_node(b_cond.m_id, h_ctx) : 0;

	ae2f_ifezerr(b_cond.m_val)		return 0;

	ae2f_ifezerr(b_cond.m_flag0 & aclcur_flag0(ACLCUR_FLAG0_VALUE))
		return 0;


	/**
	 * target labels may be forward references: they are created later,
	 * at the position the label is reached. their ref counts are then
	 * discovered by the fndjmp scan inside aclcur_cfg_lbl().
	 * */
#if	__ACLSPV_CHECK_EXTRA

	if(p_lbl_true)
		ae2f_ifnezerr_strict(p_lbl_true->m_class != ACLCUR_LBL)
			return 0;

	if(p_lbl_false)
		ae2f_ifnezerr_strict(p_lbl_false->m_class != ACLCUR_LBL)
			return 0;
#endif

	ae2f_ifezerr(id = aclcur_cfg_node(0, h_ctx)) return 0;
	ae2f_ifezerr(p = aclcur_get_node(id, h_ctx)) return 0;

	p->m_class		= ACLCUR_JC;
	p->m_d.m_jc.m_lbl_true	= c_lbl_true;
	p->m_d.m_jc.m_lbl_false = c_lbl_false;
	p->m_d.m_jc.m_brw_true	= c_brw_true;
	p->m_d.m_jc.m_brw_false = c_brw_false;
	p->m_d.m_jc.m_cond	= c_cond;
	p->m_d.m_jc.m_merge	= c_merge_opt;
	p->m_d.m_jc.m_continue	= c_continue_opt;
	p->m_d.m_jc.m_ctl	= c_ctl;

	p_lbl_true	= aclcur_get_node(c_lbl_true, h_ctx);
	p_lbl_false	= aclcur_get_node(c_lbl_false, h_ctx);
	p_cond = aclcur_get_node(c_cond, h_ctx);

	if(p_cond)	(void)++p_cond->m_num_ref;
	if(p_lbl_true)	(void)++p_lbl_true->m_num_ref;
	if(p_lbl_false)	(void)++p_lbl_false->m_num_ref;

	if(c_merge_opt) {
		h_aclcur_t p_merge = aclcur_get_node(c_merge_opt, h_ctx);
		if(p_merge)	(void)++p_merge->m_num_ref;
	}

	if(c_continue_opt) {
		h_aclcur_t p_continue = aclcur_get_node(c_continue_opt, h_ctx);
		if(p_continue)	(void)++p_continue->m_num_ref;
	}

	return id;
}

