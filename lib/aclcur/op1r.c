#include "./op.h"
#include "./util.h"

#include <ae2f/cc/pause.h>

#include <aclutil/emitx.h>

/**
 * one-operand instruction whose operand is a RAW id, not a value node.
 *
 * the normal aclcur_cfg_op1 routes its operand through aclcur_get_load,
 * which requires a node carrying ACLCUR_FLAG0_VALUE. that is right for an
 * SSA operand, but an ADDRESS has no such node: a register's id is its
 * storage address, and a register deliberately carries no VALUE flag.
 * address-of (`&x`) and the view chain both land here — the address is
 * emitted verbatim, exactly as a constant operand would be.
 *
 * the raw operand takes no reference: it is backed by no node.
 *
 * @see aclcur_cfg_op1, whose operand is validated and refcounted.
 * */
aclspv_id_t aclcur_cfg_op1r(
		aclspv_id_t	c_newid_opt,
		enum SpvOp_	c_op,
		aclspv_id_t	c_ty,
		aclspv_id_t	c_o0,
		h_aclspv_ctx_t	h_ctx
		) {
	{
		aclspv_wrd_t i;
		const aclcur_t* ae2f_restrict buf = s_get_buf_c(h_ctx);

		for(i = h_ctx->m_num_cursor; i --> 0; ae2fcc_pause()) {
			if(
					buf[i].m_class == ACLCUR_OP1
					&& buf[i].m_d.m_op.m_op	== c_op
					&& buf[i].m_ty			== c_ty
					&& buf[i].m_d.m_op.m_o[0]	== c_o0
			  ) break;
		}

		/** has one */
		if(i < h_ctx->m_num_cursor)
			return buf[i].m_id;
	}

	/** need new */
	{
		aclspv_id_t op1r = aclcur_cfg_node(c_newid_opt, h_ctx);
		h_aclcur_t p_op1r = aclcur_get_node(op1r, h_ctx);

		ae2f_ifezerr(p_op1r) return 0;

		p_op1r->m_class = ACLCUR_OP1;
		p_op1r->m_val = p_op1r->m_id;
		p_op1r->m_flag0 = aclcur_flag0(ACLCUR_FLAG0_VALUE);
		p_op1r->m_ty = c_ty;
		p_op1r->m_d.m_op.m_op = c_op;
		p_op1r->m_d.m_op.m_o[0] = c_o0;

		return p_op1r->m_id;
	}

	ae2f_unreachable();
}

/**
 * one value operand plus an immediate literal word, where the operand is a
 * RAW id rather than a value node: [op|5][ty][id][raw_o0][imm].
 *
 * this is OpAccessChain's shape. its Base is a register id — the root of a
 * view chain owns storage, and a register carries no ACLCUR_FLAG0_VALUE — so
 * the normal aclcur_cfg_op1i would reject it.
 *
 * the raw operand takes no reference unless it happens to be backed by a
 * node; when it is (a register), the node is referenced so the emit pass does
 * not drop the instruction defining it.
 * */
aclspv_id_t aclcur_cfg_op1ri(
		aclspv_id_t	c_newid_opt,
		enum SpvOp_	c_op,
		aclspv_id_t	c_ty,
		aclspv_id_t	c_o0,
		aclspv_wrd_t	c_imm,
		h_aclspv_ctx_t	h_ctx
		) {
	{
		aclspv_wrd_t i;
		const aclcur_t* ae2f_restrict buf = s_get_buf_c(h_ctx);

		for(i = h_ctx->m_num_cursor; i --> 0; ae2fcc_pause()) {
			if(
					buf[i].m_class == ACLCUR_OP2
					&& buf[i].m_d.m_op.m_op	== c_op
					&& buf[i].m_ty			== c_ty
					&& buf[i].m_d.m_op.m_o[0]	== c_o0
					&& buf[i].m_d.m_op.m_o[1]	== c_imm
			  ) break;
		}

		/** has one */
		if(i < h_ctx->m_num_cursor)
			return buf[i].m_id;
	}

	{
		aclspv_id_t op1ri = aclcur_cfg_node(c_newid_opt, h_ctx);
		h_aclcur_t p_op1ri = aclcur_get_node(op1ri, h_ctx);

		ae2f_ifezerr(p_op1ri) return 0;

		/** cfg_node regrows (reallocs) the cursor buffer: re-resolve. */
		if(c_o0) {
			h_aclcur_t p_o0 = aclcur_get_node(c_o0, h_ctx);
			if(p_o0)	(void)++p_o0->m_num_ref;
		}

		p_op1ri->m_class = ACLCUR_OP2;
		p_op1ri->m_val = p_op1ri->m_id;
		p_op1ri->m_flag0 = aclcur_flag0(ACLCUR_FLAG0_VALUE);
		p_op1ri->m_ty = c_ty;
		p_op1ri->m_d.m_op.m_op = c_op;
		p_op1ri->m_d.m_op.m_o[0] = c_o0;
		p_op1ri->m_d.m_op.m_o[1] = c_imm;

		return p_op1ri->m_id;
	}

	ae2f_unreachable();
}
