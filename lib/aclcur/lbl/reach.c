#include "./dfs.h"
#include "./reach.h"
#include "../util.h"

typedef struct {
	aclspv_bool_t	m_ret;
	aclspv_id_t	m_tar_lbl;
} s_reachable_t;

ae2f_ccpure static enum ACLCUR_LBL_DFS_ s_reachable(
		h_aclcur_t		h_child,
		ae2f_unused h_aclcur_t	h_par_opt,
		void* ae2f_restrict	rdwr_client
		) {
	s_reachable_t* ae2f_restrict p = rdwr_client;
	if(p->m_tar_lbl == h_child->m_id) {
		p->m_ret = 1;
		return ACLCUR_LBL_DFS_BREAK;
	}

	return ACLCUR_LBL_DFS_IN;
}

aclspv_bool_t aclcur_lbl_is_reachable(
		aclspv_id_t c_root,
		aclspv_id_t c_child,
		h_aclspv_ctx_t h_ctx) {
	s_reachable_t r;
	r.m_tar_lbl	= c_child;
	r.m_ret		= 0;

	aclcur_lbl_dfs(c_root, s_reachable, &r, h_ctx);

	return r.m_ret;
}

typedef struct {
	aclspv_bool_t	m_ret;
	aclspv_id_t	m_tar_lbl;
	h_aclspv_ctx_t	m_ctx;
} s_reachable_uc_t;

ae2f_ccpure static enum ACLCUR_LBL_DFS_ s_reachable_uc(
		h_aclcur_t		h_child,
		ae2f_unused h_aclcur_t	h_par_opt,
		void* ae2f_restrict	rdwr_client
		) {
	s_reachable_uc_t* ae2f_restrict p = rdwr_client;

	if(p->m_tar_lbl == h_child->m_id) {
		p->m_ret = 1;
		return ACLCUR_LBL_DFS_BREAK;
	}

	switch(s_get_buf(p->m_ctx)[h_child->m_d.m_lbl.m_nop - 1].m_class) {
		case ACLCUR_JC:
			return ACLCUR_LBL_DFS_CONTINUE;

		default:
			break;
	}

	return ACLCUR_LBL_DFS_IN;
}

aclspv_bool_t aclcur_lbl_is_reachable_uc(
		aclspv_id_t c_root,
		aclspv_id_t c_child,
		h_aclspv_ctx_t h_ctx) {
	s_reachable_uc_t r;
	r.m_tar_lbl	= c_child;
	r.m_ret		= 0;
	r.m_ctx		= h_ctx;

	aclcur_lbl_dfs(c_root, s_reachable_uc, &r, h_ctx);

	return r.m_ret;
}

/** true iff a successor of c_lbl can reach c_lbl back: the block
 * participates in a cycle (a loop block: header, body, or the continue
 * trampoline). */
aclspv_bool_t aclcur_lbl_in_cycle(
		aclspv_id_t c_lbl,
		h_aclspv_ctx_t h_ctx) {
	h_aclcur_t p_lbl = aclcur_get_node(c_lbl, h_ctx);
	h_aclcur_t p_tail;
	aclspv_wrd_t i;
	aclspv_id_t succ[4];

	ae2f_ifnezerr(!p_lbl || p_lbl->m_class != ACLCUR_LBL)
		return 0;

	p_tail = &p_lbl[p_lbl->m_d.m_lbl.m_nop - 1];

	switch(p_tail->m_class) {
		case ACLCUR_J:
			succ[0] = p_tail->m_d.m_j.m_lbl;
			i = 1;
			break;

		case ACLCUR_JC:
			succ[0] = p_tail->m_d.m_jc.m_lbl_true;
			succ[1] = p_tail->m_d.m_jc.m_lbl_false;
			i = 2;
			if(p_tail->m_d.m_jc.m_merge)
				succ[i++] = p_tail->m_d.m_jc.m_merge;
			if(p_tail->m_d.m_jc.m_continue)
				succ[i++] = p_tail->m_d.m_jc.m_continue;
			break;

		default:
			return 0;
	}

	while(i-->0) {
		/** a successor that reaches c_lbl back closes a cycle. the
		 * per-successor form avoids the trivial self-match of
		 * aclcur_lbl_is_reachable(c_lbl, c_lbl, h_ctx) (which matches
		 * the DFS root itself, i.e. every block); a self-edge counts
		 * as a cycle through the reachability root match. */
		if(succ[i]
		&& aclcur_lbl_is_reachable(succ[i], c_lbl, h_ctx))
			return 1;
	}

	return 0;
}
