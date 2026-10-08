#include "./op.h"
#include "./util.h"

#include <ae2f/cc/pause.h>

#include <aclutil/emitx.h>

/** configure a result-less two-operand instruction: [op|3][o0][o1].
 *
 * see aclcur_cfg_st for why SpvOpStore needs a shape of its own.
 *
 * @return the new node id, or 0 on failure.
 * */
aclspv_id_t aclcur_cfg_st(
		enum SpvOp_	c_op,
		aclspv_id_t	c_o0,
		aclspv_id_t	c_o1,
		h_aclspv_ctx_t	h_ctx
		) {
	aclspv_id_t st = aclcur_cfg_node(0, h_ctx);
	h_aclcur_t p_st = aclcur_get_node(st, h_ctx);

	ae2f_ifezerr(p_st) return 0;

	/** cfg_node regrows (reallocs) the cursor buffer: resolve the operands
	 * only now. */
	{
		h_aclcur_t p_o0 = c_o0 ? aclcur_get_node(c_o0, h_ctx) : 0;
		h_aclcur_t p_o1 = c_o1 ? aclcur_get_node(c_o1, h_ctx) : 0;

		p_st->m_class		= ACLCUR_ST;
		/** no result: no value and no type of its own. */
		p_st->m_val		= 0;
		p_st->m_ty		= 0;
		p_st->m_d.m_op.m_op	= c_op;
		p_st->m_d.m_op.m_o[0]	= c_o0;
		p_st->m_d.m_op.m_o[1]	= c_o1;

		/** the operands are value nodes: reference them so the emit pass
		 * does not drop the instruction they came from. a raw id backed by
		 * no node simply takes no reference. */
		if(p_o0)	(void)++p_o0->m_num_ref;
		if(p_o1)	(void)++p_o1->m_num_ref;
	}

	return p_st->m_id;
}
