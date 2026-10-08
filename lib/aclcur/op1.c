#include "./op.h"
#include "./load/get.h"
#include "./util.h"

#include <ae2f/cc/pause.h>

#include <aclconst/specop.h>
#include <aclutil/emitx.h>

aclspv_id_t aclcur_cfg_op1(
		aclspv_id_t	c_newid_opt,
		enum SpvOp_	c_op,
		aclspv_id_t	c_ty,
		aclspv_id_t	c_o0,
		h_aclspv_ctx_t	h_ctx
		) {
	aclcur_t op0 = aclcur_get_load(c_o0, h_ctx);
	h_aclcur_t p_o0;
	ae2f_ifezerr(op0.m_val) return 0;

	/** constant operands are permitted: their ids are emitted as raw operands. */
	p_o0 = op0.m_id ? aclcur_get_node(op0.m_id, h_ctx) : 0;

#if	__ACLSPV_CHECK_EXTRA
	ae2f_ifnezerr(op0.m_id && !p_o0) {
		err_puts(
				"operand node missing on aclcur op1.\n"
				"it should be handled on aclemit.\n"
				);
		h_ctx->m_err = ACLSPV_COMPILE_UNHANDLED;
		return 0;
	}
#else
	ae2f_assume_strict(!op0.m_id || p_o0);
#endif

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
		aclspv_id_t op1 = aclcur_cfg_node(c_newid_opt, h_ctx);
		h_aclcur_t p_op1 = aclcur_get_node(op1, h_ctx);

		/** cfg_node regrows (reallocs) the cursor buffer: re-resolve the operand. */
		p_o0 = op0.m_id ? aclcur_get_node(op0.m_id, h_ctx) : 0;

		p_op1->m_class = ACLCUR_OP1;
		p_op1->m_val = p_op1->m_id;
		p_op1->m_flag0 = aclcur_flag0(ACLCUR_FLAG0_VALUE);
		p_op1->m_ty = c_ty;
		p_op1->m_d.m_op.m_op = c_op;
		p_op1->m_d.m_op.m_o[0] = c_o0;

		if(p_o0)	(void)++p_o0->m_num_ref;

		return p_op1->m_id;
	}

	ae2f_unreachable();
}
