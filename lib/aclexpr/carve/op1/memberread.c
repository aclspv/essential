#include "./memberread.h"

#include <aclexpr/carve/util.h>

#include <aclty.h>
#include <aclcur/common.h>
#include <aclcur/op.h>

#include <util/constant.h>

aclexpr_carve_lbl_t aclexpr_carve1_memberread(
		struct aclexpr_visit_data* const ae2f_restrict	rdwr_data,
		aclspv_wrd_t* const ae2f_restrict rdwr_prv_idval,
		aclexpr_opbincfg_extra* const ae2f_restrict rdwr_prv_extra,
		ae2f_unused h_scale_t			h_lst_scale,
		aclspv_wrd_t* const ae2f_restrict	rdwr_lst_scale_buf,
		aclexpr_opbincfg_extra* ae2f_restrict rdwr_lst_extra,
		ae2f_unused aclutil_literal* ae2f_restrict r_ltrl,
		aclexpr_carve_lbl_t			c_lbl_continue,
		aclexpr_carve_lbl_t			c_lbl_success,
		aclexpr_carve_lbl_t			c_lbl_err
		) {
	const aclspv_id_t V = rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_END + 0];
	const aclspv_wrd_t IDX = (aclspv_wrd_t)rdwr_lst_extra[1].m_literal.m_u64;
	const aclspv_id_t TY = rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_RESTYID];

	unless(rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_OPCODE] == (aclspv_wrd_t)ACLEXPR_OP1_MEMBERREAD)
		return c_lbl_continue;

	/** the base must be a struct/union value with the indexed member. */
	{
		const h_aclcur_t P_BASE = aclcur_get_node(V, CTX(rdwr_data));
		aclspv_id_t TY_BASE = P_BASE ? P_BASE->m_ty : 0;

		/** a constant composite (the union/struct initializer's
		 * OpConstantComposite) has no aclcur cursor node: fall back
		 * to the operand's recorded type (set by OP0_VAL). */
		ae2f_ifezerr(TY_BASE)
			TY_BASE = rdwr_lst_extra[0].m_type_original;

		{
			const aclty_t* const ae2f_restrict P_TY
				= TY_BASE ? aclty_find(TY_BASE, CTX(rdwr_data)) : 0;

			ae2f_ifnezerr_strict(
					!P_TY || P_TY->m_class != ACLTY_STRUCT
					|| IDX >= P_TY->m_info.m_struct.m_num_mbr) {
				err_puts("[carve1::memberread] base is not a composite value with that member");
				return c_lbl_err;
			}
		}
	}

	ae2f_ifezerr(*rdwr_prv_idval = aclcur_cfg_op1i(
				0
				, SpvOpCompositeExtract
				, TY
				, V
				, IDX
				, CTX(rdwr_data)
				)) {
		err_puts("[carve1::memberread] OpCompositeExtract failed");
		return c_lbl_err;
	}

	*rdwr_prv_extra = rdwr_lst_extra[0];
	rdwr_prv_extra->m_type_original = TY;

	return c_lbl_success;
}

