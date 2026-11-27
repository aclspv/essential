#include "./store.h"
#include "./load/get.h"
#include "./common.h"

#include <aclty.h>

#include <util/log.h>

/** has this register ever been the target of a store? */
static aclspv_bool_t s_ever_stored(aclspv_id_t c_reg, h_aclspv_ctx_t h_ctx) {
	aclspv_wrd_t	i;

	for(i = 0; i < h_ctx->m_num_cursor; ++i) {
		h_aclcur_t p = aclcur_get_node_idx(i, h_ctx);

		if(p && p->m_class == ACLCUR_STORE
				&& p->m_d.m_store.m_reg == c_reg)
			return 1;
	}

	return 0;
}

aclspv_bool_t aclcur_store_clobber_reachable(
		aclspv_id_t		c_written,
		h_aclspv_ctx_t		h_ctx
		) {
	aclspv_wrd_t		i;
	const aclspv_wrd_t	c_n = h_ctx ? h_ctx->m_num_cursor : 0;
	aclspv_id_t		c_var = 0;

	ae2f_ifezerr(h_ctx) return 0;
	ae2f_ifezerr(h_ctx->m_cursors.m_p) return 0;

	/** WHICH var was written. a write names one — through a view, through the
	 * var named by a pointer value, or (0) through a pointer nobody can
	 * follow. this is the whole filter: no type, no storage class, no overlap
	 * guess. */
	if(c_written)
		c_var = aclcur_reg_var(c_written, h_ctx);

	/** snapshot: creating nodes below moves the cursor buffer. */
	for(i = 0; i < c_n; ++i) {
		h_aclcur_t p = aclcur_get_node_idx(i, h_ctx);
		aclspv_id_t	c_reg;
		aclspv_id_t	c_val;

		if(!p || p->m_class != ACLCUR_REG)
			continue;

		c_reg = p->m_id;

		/** the write that just happened is this register's own value: the
		 * store path already re-tracked it, and re-reading it from memory
		 * would only add a load. */
		if(c_reg == c_written)
			continue;

		/** a parameter is an SSA value, and a pure alias has no var of its
		 * own: neither holds a value a write can invalidate. nor does an
		 * interface variable: its value is its own address, which no write
		 * changes. */
		if(p->m_flag0 & (aclcur_flag0_reg(ACLCUR_FLAG0_REG_PARAM)
					| aclcur_flag0_reg(ACLCUR_FLAG0_REG_ALIAS)
					| aclcur_flag0_reg(ACLCUR_FLAG0_REG_ADDRESS)))
			continue;

		if(c_var) {
			/** the var this register names: the same one, every cached value
			 * of it is stale (the written view, its root, a sibling member, a
			 * write-through pointer to the same object). a KNOWN other var
			 * cannot have been reached by a write that names this one. an
			 * unknown one (0: "wherever that pointer points") still can. */
			const aclspv_id_t c_reg_var
				= aclcur_reg_var(c_reg, h_ctx);

			if(c_reg_var && c_reg_var != c_var)
				continue;
		}

		/** a var that can hold no memory (a plain local whose address never
		 * escaped) is never observable through a write: leave its value
		 * alone. untracking one would be worse than useless — its value lives
		 * only in the cache/SSA definition, and the later read would fall to
		 * a memory load of a var that was never written. */
		unless(aclcur_reg_needs_memory(c_reg, h_ctx))
			continue;

		if(!(p->m_flag0 & aclcur_flag0_reg(ACLCUR_FLAG0_REG_TRACKED))
				&& !s_ever_stored(c_reg, h_ctx)) {
			aclcur_reg_untrack(c_reg, h_ctx);
			continue;
		}

		/** creating nodes may move the cursor buffer: re-fetch by id. */
		aclcur_reg_untrack(c_reg, h_ctx);

		ae2f_ifezerr(p = aclcur_get_node(c_reg, h_ctx))
			continue;

		/** the tracked value predates the write, and the SSA value flow would
		 * hand it back: mark the register changed so the read below (and every
		 * later one in this block) comes from memory. */
		p->m_flag0 |= aclcur_flag0_reg(ACLCUR_FLAG0_REG_CHANGED);

		ae2f_ifezerr(c_val = aclcur_load_reg(c_reg, h_ctx, 0))
			return 0;

		/** the register's value for the rest of this block. no node is
		 * created for the store itself, so nothing refers to the load here: a
		 * read does, and the emitter drops it otherwise. */
		ae2f_ifezerr(aclcur_store_reg_implicit(c_reg, c_val, h_ctx))
			return 0;
	}

	return 1;
}

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
		p_reg->m_flag0 &= (aclspv_wrd_t)~(aclcur_flag0_reg(ACLCUR_FLAG0_REG_TRACKED)
					| aclcur_flag0_reg(ACLCUR_FLAG0_REG_CHANGED));
		p_reg->m_val	= 0;
		p_reg->m_val_mask = ACLTRL_RT;
	} else {
		p_reg->m_flag0 |= aclcur_flag0_reg(ACLCUR_FLAG0_REG_TRACKED)
		                | aclcur_flag0_reg(ACLCUR_FLAG0_REG_CHANGED);
		p_reg->m_val = c_val;
		p_reg->m_val_mask = p_val->m_val_mask;
		p_reg->m_val_lit = p_val->m_val_lit;
	}

	{
		const aclspv_id_t ID = p->m_id;

		/** the write may have landed in a var this node does not own: a store
		 * through a pointer VALUE writes the var that pointer names, and a
		 * store to one object is observable through a pointer taken from it
		 * earlier. every register naming that var drops its cached value —
		 * the same treatment a call site gives them, for the same reason.
		 *
		 * the alternative is an alias analysis no part of this compiler has:
		 * `i32* p = &t; *p = 3; use(t);` used to hand `t`'s pre-write value to
		 * `use`, and `t = 3; *p` handed the pointer the pre-write one, both
		 * silently. */
		ae2f_ifezerr(aclcur_store_clobber_reachable(c_reg, h_ctx))
			return 0;

		return ID;
	}
}

aclspv_id_t	aclcur_store_reg_implicit(
		aclcur_regid_t c_reg,
		aclspv_vid_t c_val,
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
