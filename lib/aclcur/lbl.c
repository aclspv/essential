#include "./lbl.h"
#include "./util.h"
#include "./reg.h"

#include <aclmem.h>

#include <util/log.h>

aclspv_id_t ae2f_ccpure aclcur_get_lbl_last(h_aclspv_ctx_t h_ctx) {
	h_aclcur_t r = aclcur_get_lbl_last1(h_ctx);
	return r ? r->m_id : 0;
}


struct aclcur* ae2f_ccpure aclcur_get_lbl_last1(h_aclspv_ctx_t h_ctx) {
	aclspv_wrd_t i;
	const aclspv_wrd_t c = h_ctx->m_num_cursor;

	for(i = 0; i < c; ++i) {
		h_aclcur_t p = &s_get_buf(h_ctx)[i];
		if(p->m_class == ACLCUR_LBL)
			break;
	}

	while(i < c) {
		h_aclcur_t p = &s_get_buf(h_ctx)[i];
		aclspv_wrd_t i_new = i + p->m_d.m_lbl.m_nop;

		ae2f_ifnezerr_strict(i_new <= i) {
			h_ctx->m_err = ACLSPV_COMPILE_TOO_BIG_CUR_LBL;
			return 0;
		}

		if(i_new == c)
			return p;

		if(i_new > c)
			return 0;

		ae2f_ifnezerr_strict(s_get_buf(h_ctx)[i_new].m_class != ACLCUR_LBL) {
			return p;
		}

		i = i_new;
	}

	return 0;
}

/** count every J / JC edge targeting the new label across the whole
 * buffer, including edges from blocks that are dead themselves: the
 * power-off sweep (lbl/emit.c) decrements the target of every edge it
 * elides, so each edge must be counted exactly once at creation for the
 * live/dead accounting to balance. */
static aclspv_wrd_t s_fndjmp_count(
		aclspv_id_t		c_lbl,
		h_aclspv_ctx_t		h_ctx
		) {
	aclspv_wrd_t c_num_ref = 0;
	aclspv_wrd_t i;

	for(i = 0; i < h_ctx->m_num_cursor; ++i) {
		h_aclcur_t p_edge = &s_get_buf(h_ctx)[i];

		switch(p_edge->m_class) {
			case ACLCUR_J:
				if(p_edge->m_d.m_j.m_lbl == c_lbl)
					++c_num_ref;
				break;

			case ACLCUR_JC:
				if(p_edge->m_d.m_jc.m_lbl_false == c_lbl) ++c_num_ref;
				if(p_edge->m_d.m_jc.m_lbl_true == c_lbl) ++c_num_ref;
				if(p_edge->m_d.m_jc.m_merge == c_lbl) ++c_num_ref;
				if(p_edge->m_d.m_jc.m_continue == c_lbl) ++c_num_ref;
				break;

			default:
				break;
		}
	}

	return c_num_ref;
}

aclspv_id_t aclcur_cfg_lbl(aclspv_id_t c, h_aclspv_ctx_t h_ctx) {
	aclspv_id_t p_lbl		= aclcur_cfg_node(c, h_ctx);
	h_aclcur_t p		= aclcur_get_node(p_lbl, h_ctx);
	h_aclcur_t lbl_prv;

	ae2f_ifezerr(p)		return 0;
	ae2f_ifezerr(p_lbl)	return 0;

	p->m_class		= ACLCUR_LBL;
	p->m_d.m_lbl.m_nop	= 1;


	/** 
	 * root is not bootstrapped by this.
	 * therefore, previous label is guaranteed.
	 * root of the previous label is propagated to this.
	 * */
	ae2f_ifezerr(lbl_prv = aclcur_get_node(p->m_lbl, h_ctx))
		return 0;
	p->m_d.m_lbl.m_root = lbl_prv->m_d.m_lbl.m_root;

	/** count the incoming edges to the new label. */
	p->m_num_ref += s_fndjmp_count(p_lbl, h_ctx);

	--lbl_prv->m_d.m_lbl.m_nop;

	/** a new block starts here: nothing of the register value cache carries
	 * over. a value tracked in a previous block must not be reused inside
	 * this one (a load emitted in a block that does not dominate this one
	 * would be referenced -> dominance violation). untrack() clears the
	 * TRACKED flag together with m_val/m_val_mask/m_val_lit, preserving the
	 * invariant load.c relies on: TRACKED clear ==> m_val == 0. */
	{
		aclspv_wrd_t i;
		for(i = 0; i < h_ctx->m_num_cursor; ++i) {
			h_aclcur_t p2 = &s_get_buf(h_ctx)[i];
			if(p2->m_class == ACLCUR_REG)
				aclcur_reg_untrack(p2->m_id, h_ctx);
		}
	}

	return p->m_lbl = p->m_id;
}

/** mark a label as the loop header (the while-cond block).
 * only the header takes the marker: the back edge is not carved yet
 * when its loads resolve, so no graph shape reveals the loopness then.
 * everything else (the body, the store-side and phi-side decisions)
 * is derived from graph structure: the loop JC's continue target at
 * carve time, and emit-time cycle membership. */
aclspv_id_t aclcur_cfg_lbl_loop(aclspv_id_t c_lbl, h_aclspv_ctx_t h_ctx) {
	h_aclcur_t p = aclcur_get_node(c_lbl, h_ctx);

	ae2f_ifezerr(p)		return 0;
	if(p->m_class != ACLCUR_LBL)
		return 0;

	p->m_flag0 |= aclcur_flag0_lbl(ACLCUR_FLAG0_LBL_LOOP);
	return c_lbl;
}

aclspv_id_t aclcur_cfg_root(aclspv_id_t c_retty, h_aclspv_ctx_t h_ctx) {
	h_aclcur_t p;

	ae2f_ifezerr(h_ctx)
		return 0;

	/** the root must occupy node 0: see struct aclcur::m_ty. */
	ae2f_ifnezerr_strict(h_ctx->m_num_cursor)
		return 0;

	/** replicate the append of aclcur_cfg_node2, minus the label requirement. */
	ae2f_ifnezerr(s_aclmem_cgrow2(
				&h_ctx->m_cursors
				, 1
				, sizeof(aclcur_t)
				, ACLSPV_COMPILE_ALLOC_FAILED_CURSOR))
		return 0;
	h_ctx->m_num_cursor = 1;

	/** the buffer must be live from here on. */
	ae2f_ifezerr(s_get_buf(h_ctx))
		return 0;

	p = &s_get_buf(h_ctx)[0];
	aclmemzero(p, sizeof(*p));

	p->m_class		= ACLCUR_LBL;
	p->m_flag0		= aclcur_flag0_lbl(ACLCUR_FLAG0_LBL_ROOT);
	p->m_id			= h_ctx->m_id++;
	p->m_d.m_lbl.m_nop	= 1;
	p->m_d.m_lbl.m_root	= p->m_id;
	p->m_lbl		= p->m_id;
	p->m_ty			= c_retty;
	p->m_num_ref		= 1;

	return p->m_id;
}
