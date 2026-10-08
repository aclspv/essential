#include "./phi.h"
#include "./load.h"
#include "./util.h"

#include <aclutil/emitx.h>

aclspv_bool_t aclcur_cfg_load_phi(
		aclspv_id_t c_load,
		aclspv_id_t c_lbl,
		h_aclspv_ctx_t h_ctx
		) {
	h_aclcur_t	p_load;
	aclspv_id_t	id_reg, id_store;

	p_load = aclcur_get_node(c_load, h_ctx);
	/** a node-less value id (e.g. a constant served straight by the value
	 * flow) has no memory load to attach: there is nothing to phi. */
	ae2f_ifezerr(p_load)	return 0;
	ae2f_ifnezerr_strict(p_load->m_class != ACLCUR_LOAD)
		return 0;

	id_reg = p_load->m_d.m_load.m_reg;

	id_store = aclcur_get_node_store(id_reg, c_lbl, h_ctx);
	ae2f_ifezerr(id_store)	return 0;
	p_load = aclcur_get_node(c_load, h_ctx);

	unless(p_load->m_d.m_load.m_num_phi) {
		h_aclcur_t	p_phi;
		aclspv_id_t	id_phi;

		id_phi = aclcur_cfg_node2(0, h_ctx, 1);
		ae2f_ifezerr(id_phi) return 1;

		p_phi = aclcur_get_node(id_phi, h_ctx);
		p_load = aclcur_get_node(c_load, h_ctx);

		p_phi->m_val		= id_store;
		p_phi->m_class		= ACLCUR_PHI;
		p_phi->m_d.m_phi.m_lbl	= c_lbl;
		p_phi->m_d.m_phi.m_nxt	= 0;

		p_load->m_d.m_load.m_phi	= id_phi;
		p_load->m_d.m_load.m_num_phi	= 1;

		return 0;
	}

	{
		aclspv_id_t	id_phi = p_load->m_d.m_load.m_phi;
		aclspv_id_t	id_phi_prv = 0;

		while(id_phi) {
			h_aclcur_t p_phi = aclcur_get_node(id_phi, h_ctx);
			if(p_phi->m_d.m_phi.m_lbl == c_lbl)
				return 0;

			id_phi_prv	= id_phi;
			id_phi		= p_phi->m_d.m_phi.m_nxt;
		}

		{
			h_aclcur_t p_phi_prv, p_phi;
			aclspv_id_t new_phi = aclcur_cfg_node2(0, h_ctx, 1);

			ae2f_ifezerr(new_phi) return 1;
			p_phi = aclcur_get_node(new_phi, h_ctx);

			p_phi->m_val		= id_store;
			p_phi->m_class		= ACLCUR_PHI;
			p_phi->m_d.m_phi.m_lbl	= c_lbl;
			p_phi->m_d.m_phi.m_nxt	= 0;

			p_phi_prv = aclcur_get_node(id_phi_prv, h_ctx);
			p_load = aclcur_get_node(c_load, h_ctx);
			(void)++p_load->m_d.m_load.m_num_phi;
			p_phi_prv->m_d.m_phi.m_nxt = new_phi;
		}

		return 0;
	}
}

aclspv_id_t aclcur_phi_head(aclspv_id_t c_phi, h_aclspv_ctx_t h_ctx) {
	h_aclcur_t p = aclcur_get_node(c_phi, h_ctx);
	aclspv_id_t r = c_phi;
	aclspv_id_t c_lbl;

	ae2f_ifezerr(p)		return 0;
	if(p->m_class != ACLCUR_PHI)
		return 0;

	c_lbl = p->m_lbl;

	/** walk up the m_nxt references until nothing references r anymore. */
	for(;;) {
		aclspv_id_t r_prv = r;
		aclspv_wrd_t i;

		for(i = 0; i < h_ctx->m_num_cursor; ++i) {
			h_aclcur_t q = &s_get_buf(h_ctx)[i];
			if(q->m_class == ACLCUR_PHI
			&& q->m_lbl == c_lbl
			&& q->m_d.m_phi.m_nxt == r)
				r = q->m_id;
		}

		if(r == r_prv)
			return r;
	}
}

aclspv_compile_t aclcur_phi_emit(aclspv_id_t c_head, h_aclspv_ctx_t h_ctx) {
	h_aclcur_t p = aclcur_get_node(c_head, h_ctx);
	aclspv_id_t idc;
	aclspv_wrd_t n = 0;
	aclspv_wrd_t anch;
	aclspv_wrd_t i;
	aclspv_wrd_t* wr;

	ae2f_ifezerr(p)		return ACLSPV_COMPILE_UNHANDLED;
	ae2f_ifnezerr_strict(p->m_class != ACLCUR_PHI)
		return ACLSPV_COMPILE_UNHANDLED;

	/** walk the chain: count the operands. */
	idc = c_head;
	while(idc) {
		h_aclcur_t q = aclcur_get_node(idc, h_ctx);

		ae2f_ifezerr(q)		return ACLSPV_COMPILE_UNHANDLED;
		ae2f_ifnezerr_strict(q->m_class != ACLCUR_PHI)
			return ACLSPV_COMPILE_UNHANDLED;

		++n;
		idc = q->m_d.m_phi.m_nxt;
	}

	if(!n)
		return ACLSPV_COMPILE_UNHANDLED;

	/** OpPhi layout: [opcode|field] [type] [result] (value label)*n */
	anch = h_ctx->m_count.m_fnimpl;
	ae2f_ifezerr(h_ctx->m_count.m_fnimpl = aclutil_emit_wrd(
				&h_ctx->m_section.m_fnimpl
				, h_ctx->m_count.m_fnimpl + 2 + 2 * n
				, 0
				)) return ACLSPV_COMPILE_ALLOC_FAILED_CURSOR;

	wr	= &aclutil_get_wrd_of_vec(&h_ctx->m_section.m_fnimpl)[anch];
	wr[0]	= aclspv_opcode_to_wrd(SpvOpPhi, 2 + 2 * n);
	wr[1]	= p->m_ty;
	wr[2]	= c_head;

	idc = c_head;
	for(i = 0; i < n; ++i) {
		h_aclcur_t q = aclcur_get_node(idc, h_ctx);

		wr[3 + 2 * i]		= q->m_d.m_phi.m_val;
		wr[3 + 2 * i + 1]	= q->m_d.m_phi.m_lbl;
		idc			= q->m_d.m_phi.m_nxt;
	}

	return ACLSPV_COMPILE_OK;
}

