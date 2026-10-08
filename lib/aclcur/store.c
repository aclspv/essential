#include "./store.h"
#include "./load/get.h"
#include "./common.h"

#include <util/log.h>

aclspv_id_t	aclcur_store_reg(aclspv_id_t c_reg, aclspv_id_t c_val, h_aclspv_ctx_t h_ctx) {
	aclspv_id_t node;
	aclspv_id_t c_ref;
	aclcur_t b_val;
	h_aclcur_t p;
	h_aclcur_t p_reg;
	h_aclcur_t p_val;
	h_aclcur_t p_old;

	/** the caller may have passed an alias: it is referenced, too. */
	c_ref	= c_reg;

	/** alias register has no storage: store to what it really is. */
	c_reg = aclcur_reg_resolve(c_reg, h_ctx);
	ae2f_ifezerr(c_reg)	return 0;

	node	= aclcur_cfg_node(0, h_ctx);
	b_val	= aclcur_get_load(c_val, h_ctx);
	p	= aclcur_get_node(node, h_ctx);
	p_reg	= aclcur_get_node(c_reg, h_ctx);
	p_val	= b_val.m_id ? aclcur_get_node(b_val.m_id, h_ctx) : &b_val;

	ae2f_ifezerr(p)			return 0;
	ae2f_ifezerr(p_reg)		return 0;
	ae2f_ifezerr(p_val)		return 0;
	ae2f_ifezerr(p_val->m_val)	return 0;

	ae2f_ifnezerr_strict(p_reg->m_class != ACLCUR_REG)
		return 0;

	ae2f_ifezerr(p_val->m_flag0 & aclcur_flag0(ACLCUR_FLAG0_VALUE))
		return 0;

	ae2f_ifnezerr_strict(p_reg->m_ty != p_val->m_ty)
		return 0;

	/** a register never loaded (or already untracked) has no old value to release.
	 * the old tracked value may also be a node-less constant (stored straight
	 * from m_types): such a value never acquired a reference, so skip it. */
	if(p_reg->m_val) {
		p_old = aclcur_get_node(p_reg->m_val, h_ctx);
		if(p_old)
			(void)--p_old->m_num_ref;
	}

	p->m_class		= ACLCUR_STORE;
	p->m_d.m_store.m_reg	= c_reg;
	p->m_val		= c_val;

	(void)++p_reg->m_num_ref;
	(void)++p_val->m_num_ref;

	/** the alias the store went through is a reference on it, too. */
	if(c_ref != c_reg)
		(void)++aclcur_get_node(c_ref, h_ctx)->m_num_ref;

	/** memory now holds c_val: ancestors and descendants of the written
	 * region no longer match memory, but the written register itself is
	 * exactly c_val — re-track it within the current block. the label
	 * boundary sweep (aclcur_cfg_lbl) invalidates it again at the next
	 * branch, where the phi machinery takes over. */
	aclcur_reg_untrack(c_reg, h_ctx);

	ae2f_ifezerr(p_reg = aclcur_get_node(c_reg, h_ctx))
		return 0;

	p_reg->m_flag0 |= aclcur_flag0_reg(ACLCUR_FLAG0_REG_TRACKED)
	                | aclcur_flag0_reg(ACLCUR_FLAG0_REG_CHANGED);
	p_reg->m_val = c_val;
	p_reg->m_val_mask = p_val->m_val_mask;
	p_reg->m_val_lit = p_val->m_val_lit;

	return p->m_id;
}

aclspv_id_t	aclcur_store_reg_implicit(
		aclspv_id_t c_reg,
		aclspv_id_t c_val,
		h_aclspv_ctx_t h_ctx
		) {
	h_aclcur_t p_reg;
	aclcur_t b_val;
	h_aclcur_t p_val;

	c_reg = aclcur_reg_resolve(c_reg, h_ctx);
	ae2f_ifezerr(c_reg)	return 0;

	p_reg	= aclcur_get_node(c_reg, h_ctx);
	b_val	= aclcur_get_load(c_val, h_ctx);
	p_val	= b_val.m_id ? aclcur_get_node(b_val.m_val, h_ctx) : &b_val;
	ae2f_ifezerr(p_reg)					return 0;
	ae2f_ifezerr(p_val)				return 0;
	ae2f_ifezerr(p_val->m_val)			return 0;

	ae2f_ifnezerr_strict(p_reg->m_class != ACLCUR_REG)
		return 0;
	ae2f_ifezerr(p_val->m_flag0 & aclcur_flag0(ACLCUR_FLAG0_VALUE)) {
		err_puts("ACLCUR_FLAG0_VALUE\n");
		return 0;
	}

	ae2f_ifnezerr_strict(p_reg->m_ty != p_val->m_ty)
		return 0;

	if(p_reg->m_val != p_val->m_val)
		p_reg->m_flag0 |= aclcur_flag0_reg(ACLCUR_FLAG0_REG_CHANGED);

	/** ancestors and descendants are stale; the register itself is re-tracked below. */
	aclcur_reg_untrack(c_reg, h_ctx);

	ae2f_ifezerr(p_reg = aclcur_get_node(c_reg, h_ctx))
		return 0;

	p_reg->m_flag0 |= aclcur_flag0_reg(ACLCUR_FLAG0_REG_TRACKED)
	            | aclcur_flag0_reg(ACLCUR_FLAG0_REG_CHANGED);
	p_reg->m_val = p_val->m_val;
	p_reg->m_val_mask = p_val->m_val_mask;
	return p_reg->m_id;
}

aclspv_id_t	aclcur_store_reg_explicit(
		aclspv_id_t c_reg,
		aclspv_id_t c_val,
		h_aclspv_ctx_t h_ctx
		) {
	aclspv_id_t store = aclcur_store_reg(c_reg, c_val, h_ctx);
	aclcur_t* ae2f_restrict p = aclcur_get_node(store, h_ctx);
	aclcur_t* ae2f_restrict p_reg = aclcur_get_node(c_reg, h_ctx);
	ae2f_ifezerr(p)	return 0;

	p->m_flag0 |= aclcur_flag0_store(ACLCUR_FLAG0_STORE_EXPLICIT);
	p_reg->m_flag0 &= ~aclcur_flag0_reg(ACLCUR_FLAG0_REG_TRACKED);
	p_reg->m_val = 0;
	p_reg->m_val_mask = ACLUTIL_LITERAL_RT;

	return store;
}

ae2f_ccpure aclspv_id_t	aclcur_get_node_store(
		aclspv_id_t c_reg,
		aclspv_id_t c_lbl,
		h_aclspv_ctx_t h_ctx
		) {
	h_aclcur_t p_reg;
	h_aclcur_t p_lbl;
	aclspv_wrd_t idx;

	/** stores are configured on the real register only. */
	c_reg = aclcur_reg_resolve(c_reg, h_ctx);
	ae2f_ifezerr(c_reg)	return 0;

	p_reg = aclcur_get_node(c_reg, h_ctx);
	p_lbl = aclcur_get_node(c_lbl, h_ctx);

	ae2f_ifezerr(p_reg)	return 0;
	ae2f_ifezerr(p_lbl)	return 0;

	ae2f_ifnezerr_strict(p_reg->m_class != ACLCUR_REG)
		return 0;

	ae2f_ifnezerr_strict(p_lbl->m_class != ACLCUR_LBL)
		return 0;

	for(idx = p_lbl->m_d.m_lbl.m_nop; idx --> 0; ) {
		if(p_lbl[idx].m_class == ACLCUR_STORE && p_lbl[idx].m_d.m_store.m_reg == c_reg) {
			return p_lbl[idx].m_id;
		}
	}

	return 0;
}
