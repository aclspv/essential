#include "./store.h"
#include "./load/get.h"
#include "./common.h"

#include <aclty.h>

#include <util/log.h>

aclspv_id_t	aclcur_store_reg(aclcur_regid_t c_reg, aclspv_vid_t c_val, h_aclspv_ctx_t h_ctx) {
	aclspv_id_t node;
	aclspv_id_t c_ref;
	aclcur_t b_val;
	aclspv_bool_t c_mem;
	h_aclcur_t p;
	h_aclcur_t p_reg;
	h_aclcur_t p_val;

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

	/** validation (uniform across register kinds): a register whose value
	 * is observable outside the SSA scope must reach memory, so its store
	 * is explicit whichever entry point the caller used. resolved from the
	 * storage root (aclcur_reg_needs_memory); the emit gate re-checks it
	 * (aclcur_regvar_emit_storeof) so an address taken after this store was
	 * carved still forces its OpStore out. */
	c_mem = aclcur_reg_needs_memory(c_reg, h_ctx);

	/** the register's tracked value is only a cache of a value some node
	 * already owns: a store operand, an op result, a phi. tracking takes
	 * no reference, so replacing it must release none. decrementing here
	 * would cancel the reference of the instruction that produced the
	 * value, and the emit pass drops a LOAD whose count reaches zero —
	 * leaving the store that still names it dangling. (an untracked old
	 * value also has no node at all: a constant served from m_types.) */

	p->m_class		= ACLCUR_STORE;
	p->m_d.m_store.m_reg	= c_reg;
	p->m_val		= c_val;
	if(c_mem)
		p->m_flag0 |= aclcur_flag0_store(ACLCUR_FLAG0_STORE_EXPLICIT);

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

	if(c_mem) {
		/** memory is authoritative for a non-scoped register: memory may
		 * have changed under us, so no SSA value is tracked. */
		p_reg->m_flag0 &= (aclspv_wrd_t)~aclcur_flag0_reg(ACLCUR_FLAG0_REG_TRACKED);
		p_reg->m_val	= 0;
		p_reg->m_val_mask = ACLTRL_RT;
	} else {
		p_reg->m_flag0 |= aclcur_flag0_reg(ACLCUR_FLAG0_REG_TRACKED);
		p_reg->m_val = c_val;
		p_reg->m_val_mask = p_val->m_val_mask;
		p_reg->m_val_lit = p_val->m_val_lit;
	}

	{
		const aclspv_id_t ID = p->m_id;
		const aclspv_id_t c_var = aclcur_reg_var(c_reg, h_ctx);

		/** the write belongs to the VAR: bumping its generation is the whole
		 * invalidation. every other register that names the same var recorded
		 * the old generation, so it reloads when it is next read — nothing is
		 * walked here and no load is created. a write whose var cannot be
		 * named (`*p` with a pointer parameter) invalidates everything that
		 * watches the alias generation instead. */
		aclcur_reg_gen_wrote(c_var, h_ctx);

		/** this register's cache IS the value just stored. */
		aclcur_reg_val_mark(c_reg, h_ctx);

		/** a pointer stored here names a var from now on: `i32* q = &t` makes
		 * `*q` a write into `t`, not into "somewhere". */
		{
			const aclty_t* ae2f_restrict p_ty = aclty_find(p_val->m_ty, h_ctx);

			if(p_ty && p_ty->m_class == ACLTY_PTR)
				aclcur_reg_ptr_var_store(c_reg
						, aclcur_value_var(c_val, h_ctx)
						, h_ctx);
		}

		return ID;
	}
}

aclspv_id_t	aclcur_store_reg_explicit(
		aclcur_regid_t c_reg,
		aclspv_vid_t c_val,
		h_aclspv_ctx_t h_ctx
		) {
	aclspv_id_t store = aclcur_store_reg(c_reg, c_val, h_ctx);
	aclcur_t* ae2f_restrict p = aclcur_get_node(store, h_ctx);
	aclcur_t* ae2f_restrict p_reg = aclcur_get_node(c_reg, h_ctx);
	ae2f_ifezerr(p)	return 0;

	p->m_flag0 |= aclcur_flag0_store(ACLCUR_FLAG0_STORE_EXPLICIT);
	p_reg->m_flag0 &= ~aclcur_flag0_reg(ACLCUR_FLAG0_REG_TRACKED);
	p_reg->m_val = 0;
	p_reg->m_val_mask = ACLTRL_RT;

	return store;
}

ae2f_ccpure aclspv_id_t	aclcur_get_node_store(
		aclcur_regid_t c_reg,
		aclcur_lblid_t c_lbl,
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
