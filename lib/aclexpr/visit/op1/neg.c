#include "./neg.h"
#include <util/scale.h>
#include <aclexpr/op.h>
#include <aclty.h>

#include <aclval/nil.h>

aclexpr_visit_lbl_t aclexpr_visit_op1_neg(
		const CXCursor				c_cur,
		const h_aclspv_ctx_t			h_ctx, 
		x_aclspv_vec* ae2f_restrict const	h_cmdscale,
		aclexpr_visit_lbl_t			c_lbl_success,
		aclexpr_visit_lbl_t			c_lbl_fail,
		aclexpr_visit_lbl_t			c_lbl_pass
		)
{
#if	0
	return c_lbl_pass;
	(void)c_cur;
	(void)h_ctx;
	(void)h_cmdscale;
	(void)c_lbl_success;
	(void)c_lbl_fail;
	(void)rdwr_data;
#else
	if(c_cur.kind != CXCursor_UnaryOperator)
		return c_lbl_pass;

	if(clang_getCursorUnaryOperatorKind(c_cur) != CXUnaryOperator_Minus)
		return c_lbl_pass;

	{
		aclspv_wrd_t* ae2f_restrict VEC;
		x_scale* ae2f_restrict	NEW_SCALE;

		NEW_SCALE = get_last_scale_from_vec(h_cmdscale);

		NEW_SCALE = mk_scale_from_vec(
				h_cmdscale
				, 
				aclutil_count_to_sz(ACLEXPR_OPBINCFG_END + 2)
				+ (sizeof(aclexpr_opbincfg_extra) * 2)
				);
		ae2f_ifezerr(NEW_SCALE)
			return c_lbl_fail;

		VEC = get_buf_from_scale2(aclspv_wrd_t, h_cmdscale, *NEW_SCALE);

		ae2f_ifezerr(VEC)
			return c_lbl_fail;

		VEC[ACLEXPR_OPBINCFG_NUMOPRND] = 2;			/** number of operands */
		VEC[ACLEXPR_OPBINCFG_SPECCONSTANTOP] = aclspv_opcode_to_wrd(SpvOpSpecConstantOp, 5);
		VEC[ACLEXPR_OPBINCFG_OPCODE] = (aclspv_wrd_t) ACLEXPR_OP2_SUB;
		VEC[ACLEXPR_OPBINCFG_END] = aclval_cfg_nil(ACLSPV_ID_I32, h_ctx);


		aclmemzero(&VEC[ACLEXPR_OPBINCFG_END + 2], sizeof(aclexpr_opbincfg_extra) << 1);

		{
			aclexpr_opbincfg_extra* ae2f_restrict op0 = (void* ae2f_restrict)(&VEC[(ACLEXPR_OPBINCFG_END + 2)]);
			op0->m_mask_literal = 3;
			op0->m_state0 = 0;
			op0->m_type_original = ACLSPV_ID_I32;
			op0->m_mask_literal = 3;
			op0->m_literal.m_api_uintmax = 0;
		}
	}

	return c_lbl_success;
#endif
}
