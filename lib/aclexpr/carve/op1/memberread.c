#include "./memberread.h"

#include <aclexpr/carve/util.h>
#include <aclexpr/carve/ptr.h>

#include <aclty.h>
#include <aclval.h>
#include <aclcur/common.h>
#include <aclcur/op.h>
#include <aclcur/load.h>


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

	/** ghost view lane (D6/D7): when the base is a pointer (`p->b`) or a
	 * dereference (`(*p).b`), the member is an l-value of the storage the
	 * pointer names. load it through a ghost-rooted MEMBER view rather than
	 * extracting it out of a loaded composite — and if that fails, fail
	 * rather than silently falling back: the value lane would need a struct
	 * base the pointer is not.
	 *
	 * the same view node is what the member-write carve builds, so read and
	 * write of one region name one l-value (aclcur_reg_ghost /
	 * aclcur_reg_view dedup). */
	if(aclexpr_member_base_ptr(rdwr_lst_extra[1].m_cursor, CTX(rdwr_data))) {
		const aclspv_id_t VIEW = aclexpr_member_view(
				rdwr_lst_extra[1].m_cursor
				, TY
				, (aclspv_id_t)IDX
				, CTX(rdwr_data));

		ae2f_ifezerr(VIEW) {
			err_puts("[carve1::memberread] pointer-base member view could not be built");
			return c_lbl_err;
		}

		ae2f_ifezerr(*rdwr_prv_idval = aclcur_load_reg(VIEW, CTX(rdwr_data), 0)) {
			err_puts("[carve1::memberread] view load failed");
			return c_lbl_err;
		}

		*rdwr_prv_extra = rdwr_lst_extra[0];
		rdwr_prv_extra->m_type_original = TY;
		rdwr_prv_extra->m_extra.m_view.m_reg = VIEW;
		rdwr_prv_extra->m_state0 |= (aclspv_wrd_t)ACLEXPR_OPCFGX_STATE0_LVALUE_VIEW;

		return c_lbl_success;
	}

	/** addressable-base view lane (D3/D5): when the base is a plain variable
	 * of struct/union type (`s.m`), the member is an l-value of the base's
	 * storage. build the SAME MEMBER view the member-write carve builds and
	 * load through it (aclcur_load_reg) — an OpAccessChain + OpLoad over the
	 * member rather than load-the-whole-composite + OpCompositeExtract. the
	 * view is SSA-tracked like any l-value, and a compound assignment through
	 * `s.m` stores back through exactly this node.
	 *
	 * a non-addressable base (a value-only node, a constant composite) has no
	 * storage to name, so aclexpr_member_view_reg returns 0 and it stays on
	 * the value lane below (D3). */
	{
		const aclspv_id_t VIEW = aclexpr_member_view_reg(
				rdwr_lst_extra[1].m_cursor
				, TY
				, (aclspv_id_t)IDX
				, CTX(rdwr_data));

		if(VIEW) {
			ae2f_ifezerr(*rdwr_prv_idval = aclcur_load_reg(VIEW, CTX(rdwr_data), 0)) {
				err_puts("[carve1::memberread] view load failed");
				return c_lbl_err;
			}

			*rdwr_prv_extra = rdwr_lst_extra[0];
			rdwr_prv_extra->m_type_original = TY;
			rdwr_prv_extra->m_extra.m_view.m_reg = VIEW;
			rdwr_prv_extra->m_state0
				|= (aclspv_wrd_t)ACLEXPR_OPCFGX_STATE0_LVALUE_VIEW;

			return c_lbl_success;
		}
	}

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

