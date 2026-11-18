#include "./pure.h"
#include "../lbl.h"
#include "../ssa.h"
#include "../util.h"

#include <aclty.h>

/** c_reg is a register node defined by c_lbl itself. an unknown id, a
 * non-register node, or a register owned by another label is external. */
static aclspv_bool_t s_reg_defined_in(
		aclspv_id_t		c_reg,
		aclspv_id_t		c_lbl,
		h_aclspv_ctx_t		h_ctx
		) {
	h_aclcur_t p_reg;

	ae2f_ifezerr(p_reg = aclcur_get_node(c_reg, h_ctx))
		return 0;

	return (aclspv_bool_t)(
			p_reg->m_class == ACLCUR_REG
			&& p_reg->m_lbl == c_lbl
			);
}

aclspv_compile_t aclcur_lbl_cfg_pure(aclcur_lblid_t c_lbl, h_aclspv_ctx_t h_ctx) {
	h_aclcur_t	p_lbl;
	aclcur_idx_t	idx;
	aclspv_wrd_t	i;
	aclspv_wrd_t	c_nop;

	ae2f_ifezerr(p_lbl = aclcur_get_node(c_lbl, h_ctx))
		return ACLSPV_COMPILE_UNHANDLED;
	ae2f_ifnezerr_strict(p_lbl->m_class != ACLCUR_LBL)
		return ACLSPV_COMPILE_UNHANDLED;

	idx = aclcur_idx_of(c_lbl, h_ctx);
	ae2f_ifnezerr_strict(idx == ACLCUR_IDX_INVAL)
		return ACLSPV_COMPILE_UNHANDLED;

	p_lbl->m_flag0 &= (aclspv_wrd_t)~(
			aclcur_flag0_lbl(ACLCUR_FLAG0_LBL_NOCONSTANT)
			| aclcur_flag0_lbl(ACLCUR_FLAG0_LBL_NOPURE)
			);

	/** the region is the label node itself followed by its m_nop - 1
	 * nodes; the next label (if any) sits right after them. */
	c_nop = p_lbl->m_d.m_lbl.m_nop;

	for(i = 1; i < c_nop; ++i) {
		h_aclcur_t	p;
		aclspv_id_t	c_reg;
		aclspv_wrd_t	c_bit;

		ae2f_ifezerr(p = aclcur_get_node_idx(idx + i, h_ctx))
			break;

		switch(p->m_class) {
			/** a read of an external register: the label's value is not
			 * determined by its own definitions. */
			case ACLCUR_LOAD:
				c_reg	= p->m_d.m_load.m_reg;
				c_bit	= aclcur_flag0_lbl(ACLCUR_FLAG0_LBL_NOCONSTANT);
				break;

			/** a write of an external register: the label has an effect
			 * observable after the block. */
			case ACLCUR_STORE:
				c_reg	= p->m_d.m_store.m_reg;
				c_bit	= aclcur_flag0_lbl(ACLCUR_FLAG0_LBL_NOPURE);
				break;

			default:
				continue;
		}

		unless(s_reg_defined_in(c_reg, c_lbl, h_ctx))
			p_lbl->m_flag0 |= c_bit;
	}

	return ACLSPV_COMPILE_OK;
}

aclspv_bool_t aclcur_lbl_is_const(aclcur_lblid_t c_lbl, h_aclspv_ctx_t h_ctx) {
	h_aclcur_t p_lbl;

	ae2f_ifnezerr(aclcur_lbl_cfg_pure(c_lbl, h_ctx))
		return 0;

	ae2f_ifezerr(p_lbl = aclcur_get_node(c_lbl, h_ctx))
		return 0;

	return (aclspv_bool_t)!(p_lbl->m_flag0
			& aclcur_flag0_lbl(ACLCUR_FLAG0_LBL_NOCONSTANT));
}

aclspv_bool_t aclcur_lbl_is_pure(aclcur_lblid_t c_lbl, h_aclspv_ctx_t h_ctx) {
	h_aclcur_t p_lbl;

	ae2f_ifnezerr(aclcur_lbl_cfg_pure(c_lbl, h_ctx))
		return 0;

	ae2f_ifezerr(p_lbl = aclcur_get_node(c_lbl, h_ctx))
		return 0;

	return (aclspv_bool_t)!(p_lbl->m_flag0
			& aclcur_flag0_lbl(ACLCUR_FLAG0_LBL_NOPURE));
}

/** the register's storage is owned by the current function: its storage
 * ROOT is Function storage, the only kind created with
 * ACLCUR_FLAG0_REG_SCOPED.
 *
 * an ALIAS2 view copies no policy (see aclcur_reg_view), so the chain is
 * walked to the storage owner exactly like aclcur_reg_needs_memory does.
 * pure aliases were already resolved by the load/store carving. a ghost
 * anchors on a pointer value, owns no storage, and is not SCOPED: the
 * pointee is external by definition. unknown ids and non-register nodes
 * count as external (conservative).
 *
 * a POINTER PARAMETER is the same case: the parameter is an owned SSA value,
 * but the storage a view of it reaches is the caller's, not this function's.
 * reading the parameter's own value is not an access through a view, so the
 * hop count tells them apart. */
static aclspv_bool_t s_reg_owned(aclspv_id_t c_reg, h_aclspv_ctx_t h_ctx) {
	aclspv_wrd_t	n = h_ctx ? h_ctx->m_num_cursor : 0;
	aclspv_id_t	c = c_reg;
	h_aclcur_t	p;
	aclspv_bool_t	c_view = 0;

	while(c && n--) {
		ae2f_ifezerr(p = aclcur_get_node(c, h_ctx))
			return 0;
		ae2f_ifnezerr_strict(p->m_class != ACLCUR_REG)
			return 0;

		if(!(p->m_flag0 & aclcur_flag0_reg(ACLCUR_FLAG0_REG_ALIAS2)))
			break;

		c_view = 1;
		c = p->m_d.m_reg.m_d.m_ref.m_id;
	}

	ae2f_ifezerr(c)
		return 0;
	ae2f_ifezerr(p = aclcur_get_node(c, h_ctx))
		return 0;
	ae2f_ifnezerr_strict(p->m_class != ACLCUR_REG)
		return 0;

	/** an access through a view of a parameter reaches the caller's storage. */
	if(c_view && (p->m_flag0 & aclcur_flag0_reg(ACLCUR_FLAG0_REG_PARAM)))
		return 0;

	/** a POINTER parameter is an owned SSA value, but storing through it
	 * (the register itself is the address) reaches the caller's storage. a
	 * scalar parameter stays owned. */
	if(p->m_flag0 & aclcur_flag0_reg(ACLCUR_FLAG0_REG_PARAM)) {
		aclty_t* ae2f_restrict p_ty = aclty_find(p->m_ty, h_ctx);

		if(p_ty && p_ty->m_class == ACLTY_PTR)
			return 0;
	}

	return (aclspv_bool_t)(p->m_flag0
			& aclcur_flag0_reg(ACLCUR_FLAG0_REG_SCOPED));
}

/** a LOAD of an interface register that names no view is address
 * materialization: reading `out` / `in` yields the resource pointer, not
 * its contents. the value-flow reads that count go through a GHOST or an
 * ALIAS2 view. */
static aclspv_bool_t s_load_is_addr(aclspv_id_t c_reg, h_aclspv_ctx_t h_ctx) {
	h_aclcur_t p;

	ae2f_ifezerr(p = aclcur_get_node(c_reg, h_ctx))
		return 0;

	if(p->m_class != ACLCUR_REG)
		return 0;

	return (aclspv_bool_t)!(
			(p->m_flag0 & aclcur_flag0_reg(ACLCUR_FLAG0_REG_GHOST))
			|| (p->m_flag0 & aclcur_flag0_reg(ACLCUR_FLAG0_REG_ALIAS2))
			);
}

aclspv_compile_t aclcur_fn_cfg_pure(h_aclspv_ctx_t h_ctx) {
	h_aclcur_t	p_root;
	aclspv_wrd_t	i;
	aclspv_id_t	c_lbl = 0;
	aclspv_bool_t	is_live = 0;
	aclspv_bool_t	noconstant = 0;
	aclspv_bool_t	nopure = 0;

	ae2f_ifezerr(p_root = aclcur_get_node_idx(ACLCUR_IDX_ROOT, h_ctx))
		return ACLSPV_COMPILE_UNHANDLED;
	ae2f_ifnezerr_strict(p_root->m_class != ACLCUR_LBL)
		return ACLSPV_COMPILE_UNHANDLED;

	for(i = 0; i < h_ctx->m_num_cursor; ++i) {
		h_aclcur_t	p = &s_get_buf(h_ctx)[i];
		aclspv_id_t	c_reg;

		switch(p->m_class) {
			case ACLCUR_LOAD:
				c_reg = p->m_d.m_load.m_reg;
				break;

			case ACLCUR_STORE:
				c_reg = p->m_d.m_store.m_reg;
				break;

			default:
				continue;
		}

		/** a block's nodes are contiguous, so the liveness walk runs
		 * once per block instead of once per access. a node without a
		 * label cannot be proven dead: keep it. */
		if(p->m_lbl != c_lbl) {
			c_lbl = p->m_lbl;
			is_live = c_lbl ? aclcur_ssa_lbl_live(c_lbl, h_ctx) : 1;
		}

		unless(is_live)
			continue;

		if(s_reg_owned(c_reg, h_ctx))
			continue;

		if(p->m_class == ACLCUR_STORE) {
			nopure = 1;
		}
		else {
			/** an interface root load is an address, not state. */
			unless(s_load_is_addr(c_reg, h_ctx))
				noconstant = 1;
		}

		if(noconstant && nopure)
			break;
	}

	p_root->m_d.m_lbl.m_fn_noconstant = noconstant;
	p_root->m_d.m_lbl.m_fn_nopure = nopure;

	return ACLSPV_COMPILE_OK;
}

aclspv_bool_t aclcur_fn_is_const(h_aclspv_ctx_t h_ctx) {
	h_aclcur_t p_root;

	ae2f_ifnezerr(aclcur_fn_cfg_pure(h_ctx))
		return 0;

	ae2f_ifezerr(p_root = aclcur_get_node_idx(ACLCUR_IDX_ROOT, h_ctx))
		return 0;

	return (aclspv_bool_t)!p_root->m_d.m_lbl.m_fn_noconstant;
}

aclspv_bool_t aclcur_fn_is_pure(h_aclspv_ctx_t h_ctx) {
	h_aclcur_t p_root;

	ae2f_ifnezerr(aclcur_fn_cfg_pure(h_ctx))
		return 0;

	ae2f_ifezerr(p_root = aclcur_get_node_idx(ACLCUR_IDX_ROOT, h_ctx))
		return 0;

	return (aclspv_bool_t)!p_root->m_d.m_lbl.m_fn_nopure;
}
