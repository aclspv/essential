#include "./reg.h"

#include <aclemit/inline.h>
#include "./util.h"


ae2f_ccpure aclspv_id_t	aclcur_reg_resolve(aclspv_id_t c_reg, h_aclspv_ctx_t h_ctx) {
	aclspv_wrd_t n = h_ctx ? h_ctx->m_num_cursor : 0;
	h_aclcur_t p;

	/** alias chain can never be longer than the cursor count. */
	while(n--) {
		ae2f_ifezerr(c_reg) return 0;
		ae2f_ifezerr(p = aclcur_get_node(c_reg, h_ctx)) return 0;

		ae2f_ifnezerr_strict(p->m_class != ACLCUR_REG) return 0;

		unless(p->m_flag0 & aclcur_flag0_reg(ACLCUR_FLAG0_REG_ALIAS))
			return c_reg;

		c_reg = p->m_d.m_reg.m_d.m_ref.m_id;
	}

	/** cyclic or too long alias chain */
	return 0;
}

aclspv_id_t	aclcur_reg_addr(aclspv_id_t c_reg, h_aclspv_ctx_t h_ctx) {
	h_aclcur_t p;

	c_reg = aclcur_reg_resolve(c_reg, h_ctx);
	ae2f_ifezerr(c_reg) 		return 0;
	ae2f_ifezerr(p = aclcur_get_node(c_reg, h_ctx))
		return 0;

	ae2f_ifezerr(p->m_d.m_reg.m_in.m_addr)
		return 0;

	return p->m_d.m_reg.m_in.m_addr(c_reg, h_ctx);
}

#include <ae2f/Cxx/Constexpr.h>

aclconst_specsz	aclcur_reg_size(aclspv_id_t c_reg, h_aclspv_ctx_t h_ctx) {
	ae2f_constexpr aclconst_specsz ZER = {0, };
	h_aclcur_t p;

	c_reg = aclcur_reg_resolve(c_reg, h_ctx);
	ae2f_ifezerr(c_reg)		return ZER;
	ae2f_ifezerr(p = aclcur_get_node(c_reg, h_ctx))
		return ZER;

	ae2f_ifezerr(p->m_d.m_reg.m_in.m_size)
		return ZER;

	return p->m_d.m_reg.m_in.m_size(c_reg, h_ctx);
}

aclspv_id_t	aclcur_reg_base(aclspv_id_t c_reg, h_aclspv_ctx_t h_ctx) {
	h_aclcur_t p;

	ae2f_ifezerr(p = aclcur_get_node(c_reg, h_ctx))
		return 0;
	ae2f_ifnezerr_strict(p->m_class != ACLCUR_REG)
		return 0;

	/** m_base == 0 is the real-register identity sentinel: the register's
	 * own id is its base. m_id is read live, not copied, because it may
	 * have been reassigned to a shared OpVariable result id. a ghost
	 * carries its held pointer value here instead. */
	return p->m_d.m_reg.m_base
		? p->m_d.m_reg.m_base
		: p->m_id;
}

aclspv_bool_t	aclcur_reg_needs_memory(aclspv_id_t c_reg, h_aclspv_ctx_t h_ctx) {
	const aclspv_id_t c_var = aclcur_reg_var(c_reg, h_ctx);
	h_aclcur_t p;

	/** names no var: assume memory, the safe answer. */
	ae2f_ifezerr(c_var)
		return 1;
	ae2f_ifezerr(p = aclcur_get_node(c_var, h_ctx))
		return 1;

	if(p->m_class != ACLCUR_REG)
		return 1;

	/** the VAR decides whether it must exist in memory, not any view of it:
	 * a non-SCOPED root is observable outside the SSA scope, an ALLOCATED one
	 * had its address taken, and a slice-local register lives inside an
	 * inlined body's single-iteration loop (its value returns on that loop's
	 * own back edge, so memory holds it). any of these forces the store to
	 * emit its OpStore. */
	return (aclspv_bool_t)(!(p->m_flag0 & aclcur_flag0_reg(ACLCUR_FLAG0_REG_SCOPED))
			|| (p->m_flag0 & aclcur_flag0_reg(ACLCUR_FLAG0_REG_ALLOCATED))
			|| (p->m_flag0 & aclcur_flag0_reg(ACLCUR_FLAG0_REG_INLINE_SLICE)));
}

/** deepest value -> var chain this walk follows (a copy of a copy of a chain).
 * bounded like every other chain walk here. */
#define	ACLCUR_VAR_MAX_DEPTH	8

static aclspv_id_t s_value_var(
		aclspv_id_t		c_val,
		h_aclspv_ctx_t		h_ctx,
		aclspv_wrd_t		c_depth
		);

aclspv_id_t	aclcur_reg_var(aclspv_id_t c_reg, h_aclspv_ctx_t h_ctx) {
	aclspv_wrd_t n = h_ctx ? h_ctx->m_num_cursor : 0;
	aclspv_id_t c = c_reg;
	h_aclcur_t p;

	while(c && n--) {
		ae2f_ifezerr(p = aclcur_get_node(c, h_ctx))
			return 0;
		ae2f_ifnezerr_strict(p->m_class != ACLCUR_REG)
			return 0;

		/** a view is a PART of its root: the var is the root's. */
		if(p->m_flag0 & aclcur_flag0_reg(ACLCUR_FLAG0_REG_ALIAS2)) {
			c = p->m_d.m_reg.m_d.m_ref.m_id;
			continue;
		}

		/** a ghost names whatever its held pointer value names. */
		if(p->m_flag0 & aclcur_flag0_reg(ACLCUR_FLAG0_REG_GHOST))
			return s_value_var(aclcur_reg_base(c, h_ctx), h_ctx, 0);

		/** a parameter is an SSA value, and anything else without the VAR
		 * flag owns no object: neither is something a write can land in. */
		return (p->m_flag0 & aclcur_flag0_reg(ACLCUR_FLAG0_REG_VAR)) ? c : 0;
	}

	return 0;
}

aclspv_bool_t	aclcur_reg_val_current(
		aclspv_id_t		c_reg,
		const x_aclspv_ctx*	ae2f_restrict	h_ctx
		) {
	const h_aclcur_t p = aclcur_get_node(c_reg, h_ctx);
	const aclspv_id_t c_var = aclcur_reg_var(c_reg, (h_aclspv_ctx_t)h_ctx);

	ae2f_ifezerr(p)		return 0;
	ae2f_ifezerr(h_ctx)	return 0;

	unless(p->m_flag0 & aclcur_flag0_reg(ACLCUR_FLAG0_REG_TRACKED))
		return 0;

	/** its value is its own ADDRESS (an entry point's interface variable):
	 * no write changes that, so nothing can make it stale. */
	if(p->m_flag0 & aclcur_flag0_reg(ACLCUR_FLAG0_REG_ADDRESS))
		return 1;

	if(c_var) {
		const h_aclcur_t p_var = aclcur_get_node(c_var, h_ctx);

		ae2f_ifezerr(p_var) return 0;

		/** the var was written since this value was taken? */
		if(p->m_val_gen != p_var->m_gen)
			return 0;

		/** a write this compiler could not attribute to any var may have
		 * landed here — but only where memory is the authority: a plain SSA
		 * local's value is the flow's, and no pointer can reach it. */
		return (aclspv_bool_t)(
				!aclcur_reg_needs_memory(c_reg, (h_aclspv_ctx_t)h_ctx)
				|| p->m_val_alias == h_ctx->m_gen_alias);
	}

	/** names no var: nothing here can be compared, so the alias watermark the
	 * load path checks decides (see s_load_without_phi). */
	return 1;
}

void	aclcur_reg_val_mark(aclspv_id_t c_reg, h_aclspv_ctx_t h_ctx) {
	const aclspv_id_t c_var = aclcur_reg_var(c_reg, h_ctx);
	h_aclcur_t p = aclcur_get_node(c_reg, h_ctx);

	ae2f_ifezerr(p) return;

	if(c_var) {
		const h_aclcur_t p_var = aclcur_get_node(c_var, h_ctx);

		p->m_val_gen = p_var ? p_var->m_gen : 0;
	} else p->m_val_gen = 0;

	p->m_val_alias = h_ctx->m_gen_alias;
}

/**
 * record a write that landed in `c_var` (0: a var we cannot name).
 *
 * this is the whole invalidation: every register naming the var sees a
 * generation it did not record, and reloads when it is next read. nothing is
 * walked, nothing is eagerly loaded.
 * */
void	aclcur_reg_gen_wrote(aclspv_id_t c_var, h_aclspv_ctx_t h_ctx) {
	h_aclcur_t p_var;

	ae2f_ifezerr(h_ctx) return;

	if(c_var) {
		ae2f_ifezerr(p_var = aclcur_get_node(c_var, h_ctx))
			return;

		(void)++p_var->m_gen;
		return;
	}

	/** no var: it may be any of them. */
	(void)++h_ctx->m_gen_alias;
}

/** the var a pointer VALUE names, or 0 when it names none we can follow.
 *
 * a value is nameable when it was built from a register's own address: the
 * address-of carve emits `OpCopyObject <register>`, an entry-point argument
 * load copies the variable's own address, a view's address is an
 * OpAccessChain over the root. anything else — a pointer parameter, a pointer
 * read back from a pointer variable, a cast — names nothing, and the caller
 * must be conservative.
 * */
static aclspv_id_t s_value_var(
		aclspv_id_t		c_val,
		h_aclspv_ctx_t		h_ctx,
		aclspv_wrd_t		c_depth
		) {
	h_aclcur_t p;

	if(!c_val || c_depth >= ACLCUR_VAR_MAX_DEPTH)
		return 0;

	ae2f_ifezerr(p = aclcur_get_node(c_val, h_ctx))
		return 0;

	switch(p->m_class) {
		/** the value IS a register: its id is the pointer to it. */
		case ACLCUR_REG:
			return aclcur_reg_var(c_val, h_ctx);

		/** loading a register whose read yields its own address (the entry
		 * point's interface variable) is that address. a plain memory load
		 * reads a pointer value somebody else wrote: not nameable here. */
		case ACLCUR_LOAD:
			{
				h_aclcur_t p_reg = aclcur_get_node(
						p->m_d.m_load.m_reg, h_ctx);

				if(p_reg && (p_reg->m_flag0
						& aclcur_flag0_reg(ACLCUR_FLAG0_REG_ADDRESS)))
					return aclcur_reg_var(
							p->m_d.m_load.m_reg, h_ctx);

				return 0;
			}

		/** `&x` wraps the address in OpCopyObject, exactly because a store
		 * needs a typed pointer VALUE (see aclexpr_carve1_address); unwrap it
		 * to the address itself. */
		case ACLCUR_OP1:
			return (p->m_d.m_op.m_op == SpvOpCopyObject)
				? s_value_var(p->m_d.m_op.m_o[0], h_ctx, c_depth + 1)
				: 0;

		/** OpAccessChain: the view's address is its BASE's address plus
		 * indices, so the var is the base's — a part of it, not a different
		 * one. */
		case ACLCUR_OP2:
			return (p->m_d.m_op.m_op == SpvOpAccessChain)
				? s_value_var(p->m_d.m_op.m_o[0], h_ctx, c_depth + 1)
				: 0;

		default:
			return 0;
	}
}

aclspv_id_t	aclcur_reg_alias2(
		aclspv_id_t c_reg,
		aclspv_id_t c_parent,
		h_aclspv_ctx_t h_ctx
		) {
	h_aclcur_t p_reg;
	h_aclcur_t p_parent;

	ae2f_ifezerr(p_reg = aclcur_get_node(c_reg, h_ctx))
		return 0;
	ae2f_ifezerr(p_parent = aclcur_get_node(c_parent, h_ctx))
		return 0;

	ae2f_ifnezerr_strict(p_reg->m_class != ACLCUR_REG)
		return 0;
	ae2f_ifnezerr_strict(p_parent->m_class != ACLCUR_REG)
		return 0;

	/** a self-link would loop the child list forever. */
	ae2f_ifnezerr_strict(c_reg == c_parent)
		return 0;

	p_reg->m_flag0 |= aclcur_flag0_reg(ACLCUR_FLAG0_REG_ALIAS2);
	p_reg->m_d.m_reg.m_d.m_ref.m_id	= c_parent;
	p_reg->m_d.m_reg.m_id_child_nxt	= p_parent->m_d.m_reg.m_id_child_head;
	p_parent->m_d.m_reg.m_id_child_head	= c_reg;

	/** NO eager ACLCUR_FLAG0_REG_ALLOCATED propagation: the parent is
	 * materialized only when liveness asks for it (aclcur_regvar_emit_reg
	 * follows ALIAS2 descendants) or when its address is taken
	 * (aclexpr_ptr_taken). an SSA-only view leaves no storage behind. */

	return c_reg;
}

void	aclcur_reg_untrack(aclspv_id_t c_reg, h_aclspv_ctx_t h_ctx) {
	h_aclcur_t p;
	aclspv_id_t c_self;
	aclspv_id_t idc;
	aclspv_wrd_t n;

	c_self = aclcur_reg_resolve(c_reg, h_ctx);
	ae2f_ifezerr(c_self)	return;

	/** a function parameter is an SSA value, not branch state: no label
	 * sweep may clear it. */
	ae2f_ifezerr(p = aclcur_get_node(c_self, h_ctx))
		return;

	if(p->m_flag0 & aclcur_flag0_reg(ACLCUR_FLAG0_REG_PARAM))
		return;

	/** 
	 * the register itself and every ALIAS2 ancestor no longer match memory:
	 * each hop resolves first, so a pure alias sitting between an ALIAS2
	 * view and its parent is flattened as well. bounded by cursor count,
	 * like aclcur_reg_resolve().
	 * */
	n = h_ctx ? h_ctx->m_num_cursor : 0;
	c_reg = c_self;
	while(c_reg && n--) {
		c_reg = aclcur_reg_resolve(c_reg, h_ctx);
		ae2f_ifezerr(c_reg)	break;
		ae2f_ifezerr(p = aclcur_get_node(c_reg, h_ctx))
			break;
		ae2f_ifnezerr_strict(p->m_class != ACLCUR_REG)
			break;

		p->m_flag0 &= (aclspv_wrd_t)~aclcur_flag0_reg(ACLCUR_FLAG0_REG_TRACKED);
		p->m_val	= 0;
		p->m_val_mask	= 0;
		p->m_val_lit.m_api_uintmax = 0;

		/** forgetting is not staleness: everything written so far is folded
		 * into the flow this block will resolve against, so resync the
		 * watermark rather than claiming an alias write went unseen. */
		p->m_val_alias	= h_ctx ? h_ctx->m_gen_alias : 0;

		c_reg = (p->m_flag0 & aclcur_flag0_reg(ACLCUR_FLAG0_REG_ALIAS2))
		      ? p->m_d.m_reg.m_d.m_ref.m_id
		      : 0;
	}

	/** every ALIAS2 descendant is a sub-view of the written region. */
	ae2f_ifezerr(p = aclcur_get_node(c_self, h_ctx))
		return;
	ae2f_ifnezerr_strict(p->m_class != ACLCUR_REG)
		return;

	idc = p->m_d.m_reg.m_id_child_head;
	while(idc) {
		h_aclcur_t pc = aclcur_get_node(idc, h_ctx);
		c_reg = (pc && pc->m_class == ACLCUR_REG)
		      ? pc->m_d.m_reg.m_id_child_nxt
		      : 0;

		/** children may have sub-views of their own. */
		aclcur_reg_untrack(idc, h_ctx);
		idc = c_reg;
	}
}

aclspv_id_t aclcur_cfg_reg1(
		aclspv_id_t		c_varid_opt,
		CXCursor		c_cur,
		aclspv_id_t		c_ty,
		aclspv_wrd_t		c_storage_class,
		const aclcur_reg_t*	c_in_opt,
		h_aclspv_ctx_t		h_ctx
		) {
	aclspv_id_t id;
	h_aclcur_t p;

	ae2f_ifezerr(id = aclcur_cfg_node(c_varid_opt, h_ctx))
		return 0;

	ae2f_ifezerr(p = aclcur_get_node(id, h_ctx))
		return 0;

	p->m_class			= ACLCUR_REG;
	p->m_cur			= c_cur;
	p->m_ty				= c_ty;
	p->m_d.m_reg.m_storage_class	= c_storage_class;
	if(c_in_opt)
		p->m_d.m_reg.m_in = c_in_opt->m_in;

	/** fresh identity: a real register's base is its own id (sentinel 0);
	 * m_step only becomes meaningful once the register is an ALIAS2 view. */
	p->m_d.m_reg.m_base	= 0;
	p->m_d.m_reg.m_step	= 0;

	/** SCOPED initialisation (the only varied part of the store policy):
	 * a Function-storage register's value lives in SSA and needs no
	 * memory; every other storage class is observable outside the SSA
	 * scope, so its stores must reach memory. the store path reads the
	 * flag uniformly (see aclcur_store_reg). */
	if(c_storage_class == (aclspv_wrd_t)SpvStorageClassFunction)
		p->m_flag0 |= aclcur_flag0_reg(ACLCUR_FLAG0_REG_SCOPED);

	/** a register created here OWNS storage: the substrate may drop its
	 * OpVariable as dead, but the object exists and a pointer can name it.
	 * (a view, a ghost and an emitted function's parameter are built
	 * elsewhere and carry no such claim.) */
	p->m_flag0 |= aclcur_flag0_reg(ACLCUR_FLAG0_REG_VAR);

	/** created inside an inlined body's loop scaffold: local to it. */
	if(aclemit_inline_nested(h_ctx))
		p->m_flag0 |= aclcur_flag0_reg(ACLCUR_FLAG0_REG_INLINE_SLICE);

	return id;
}
