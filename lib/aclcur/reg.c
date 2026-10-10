#include "./reg.h"
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

	/** a view implies its whole ancestor chain must be kept: the parent
	 * is materialized (OpVariable) even with no live load / explicit
	 * store. transitivity holds because every view link goes through
	 * this function — each ancestor received the flag when the view
	 * directly above it was linked. */
	p_parent->m_flag0 |= aclcur_flag0_reg(ACLCUR_FLAG0_REG_ALLOCATED);

	return c_reg;
}

void	aclcur_reg_untrack(aclspv_id_t c_reg, h_aclspv_ctx_t h_ctx) {
	h_aclcur_t p;
	aclspv_id_t c_self;
	aclspv_id_t idc;
	aclspv_wrd_t n;

	c_self = aclcur_reg_resolve(c_reg, h_ctx);
	ae2f_ifezerr(c_self)	return;

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

		p->m_flag0 &= (aclspv_wrd_t)~(aclcur_flag0_reg(ACLCUR_FLAG0_REG_TRACKED)
		                            | aclcur_flag0_reg(ACLCUR_FLAG0_REG_CHANGED));
		p->m_val	= 0;
		p->m_val_mask	= 0;
		p->m_val_lit.m_api_uintmax = 0;

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

	return id;
}
