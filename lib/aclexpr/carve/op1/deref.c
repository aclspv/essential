#include "./deref.h"

#include <aclexpr/carve/util.h>
#include <aclexpr/carve/ptr.h>

#include <aclty.h>
#include <aclcur/common.h>
#include <aclcur/op.h>

/** `*p` — OpLoad from the operand's pointer value.
 *
 * the pointer operand is re-derived from the deref cursor
 * (extra[0].m_cursor): the collected operand is the operand's *value*, and
 * the pointer we need is what that value holds. the load's Result Type is
 * the pointee of the pointer register's type.
 *
 * @warning aclcur_cfg_op1 folds an identical earlier OpLoad (same operand,
 * same type) into one instruction. That is the same caching characteristic
 * every op1 in this compiler already has, and the register load cache is no
 * stricter — it is not addressed here.
 * */
aclexpr_carve_lbl_t aclexpr_carve1_deref(
		struct aclexpr_visit_data* const ae2f_restrict	rdwr_data,
		aclspv_wrd_t* const ae2f_restrict	rdwr_prv_idval,
		aclexpr_opbincfg_extra* const ae2f_restrict rdwr_prv_extra,
		ae2f_unused h_scale_t			h_lst_scale,
		aclspv_wrd_t* const ae2f_restrict	rdwr_lst_scale_buf,
		aclexpr_opbincfg_extra* ae2f_restrict	rdwr_lst_extra,
		ae2f_unused aclutil_literal* ae2f_restrict r_ltrl,
		aclexpr_carve_lbl_t			c_lbl_continue,
		aclexpr_carve_lbl_t			c_lbl_success,
		aclexpr_carve_lbl_t			c_lbl_err
		) {
	unless(rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_OPCODE] == (aclspv_wrd_t)ACLEXPR_OP1_DEREF)
		return c_lbl_continue;

	{
		const CXCursor DC = rdwr_lst_extra[0].m_cursor;
		const aclspv_id_t REG = aclexpr_opnd_reg(DC, CTX(rdwr_data));
		aclspv_id_t TY_POI;
		aclspv_id_t PTR;

		ae2f_ifnezerr_strict(!REG) {
			err_puts("[carve1::deref] no register for the deref operand");
			return c_lbl_err;
		}

		/** classify before using it: an array here means decay has not run,
		 * anything else means it was never a pointer. either way this must
		 * be an explicit refusal, never a load through whatever the id
		 * happens to be. */
		if(!aclexpr_ptr_require(REG, CTX(rdwr_data))) {
			err_puts("[carve1::deref] the deref operand is not a pointer");
			return c_lbl_err;
		}

		ae2f_ifezerr(TY_POI = aclexpr_ptr_ty(REG, CTX(rdwr_data))) {
			err_puts("[carve1::deref] operand is not a pointer");
			return c_lbl_err;
		}

		ae2f_ifezerr(PTR = aclexpr_ptrval(REG, CTX(rdwr_data))) {
			err_puts("[carve1::deref] no pointer value for the operand");
			return c_lbl_err;
		}

		/** OpLoad: [op|4][ResultType][Result][Pointer] — the ACLCUR_OP1
		 * shape exactly. */
		ae2f_ifezerr(*rdwr_prv_idval = aclcur_cfg_op1(
					0
					, SpvOpLoad
					, TY_POI
					, PTR
					, CTX(rdwr_data)
					)) {
			err_puts("[carve1::deref] OpLoad failed");
			return c_lbl_err;
		}

		*rdwr_prv_extra = rdwr_lst_extra[0];
		rdwr_prv_extra->m_type_original = TY_POI;
		/** a load is never a compile-time literal. */
		rdwr_prv_extra->m_mask_literal = 0;
		/** hand the pointer on: a compound assignment through this lvalue
		 * (`*q += 2`) has to store back through exactly this pointer, and
		 * by the time it carves, extra[0].m_cursor has been overwritten on
		 * the way down the visit cascade. m_extra survives that. */
		rdwr_prv_extra->m_extra.m_deref.m_ptr_id = PTR;
		rdwr_prv_extra->m_state0
			|= (aclspv_wrd_t)ACLEXPR_OPCFGX_STATE0_DEREF_LOADED;
	}

	return c_lbl_success;
}

/** `&x` — the address of the operand's storage, emitted as nothing at all.
 *
 * the operand register's m_addr hook is the whole operation: identity for a
 * var (the result id IS its OpVariable / OpFunctionParameter), one freshly
 * appended OpAccessChain for a view. the pointer type is built here because
 * it needs the register's storage class.
 * */
aclexpr_carve_lbl_t aclexpr_carve1_address(
		struct aclexpr_visit_data* const ae2f_restrict	rdwr_data,
		aclspv_wrd_t* const ae2f_restrict	rdwr_prv_idval,
		aclexpr_opbincfg_extra* const ae2f_restrict rdwr_prv_extra,
		ae2f_unused h_scale_t			h_lst_scale,
		aclspv_wrd_t* const ae2f_restrict	rdwr_lst_scale_buf,
		aclexpr_opbincfg_extra* ae2f_restrict	rdwr_lst_extra,
		ae2f_unused aclutil_literal* ae2f_restrict r_ltrl,
		aclexpr_carve_lbl_t			c_lbl_continue,
		aclexpr_carve_lbl_t			c_lbl_success,
		aclexpr_carve_lbl_t			c_lbl_err
		) {
	unless(rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_OPCODE] == (aclspv_wrd_t)ACLEXPR_OP1_ADDRESS)
		return c_lbl_continue;

	/** `&a[i]`: the subscript carve has already produced the element's address
	 * — it built the view, called regview_addr, and re-typed the result as a
	 * pointer VALUE. there is nothing left for the address-of to do.
	 *
	 * this branch MUST come before the register lookup below. extra[0].m_cursor
	 * names the ARRAY here, not the subscript: the cascade rewrote that slot on
	 * the way down and the subscript carve handed its own extra[0] back
	 * wholesale. resolving it would take the address of the array base and
	 * yield ptr([3 x int]) where `int* p2` wants ptr(int) — a mismatch the
	 * arithmetic cast correctly refuses. */
	if(rdwr_lst_extra[0].m_state0
			& (aclspv_wrd_t)ACLEXPR_OPCFGX_STATE0_ADDR_OF_ELM) {
		*rdwr_prv_idval = rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_END];
		*rdwr_prv_extra = rdwr_lst_extra[0];
		return c_lbl_success;
	}

	{
		const CXCursor AC = rdwr_lst_extra[0].m_cursor;
		const aclspv_id_t REG = aclexpr_opnd_reg(AC, CTX(rdwr_data));
		const h_aclcur_t P_REG = REG
				? aclcur_get_node(REG, CTX(rdwr_data))
				: 0;
		aclspv_id_t TY_PTR;
		aclspv_id_t PTR;

		ae2f_ifnezerr_strict(!P_REG || P_REG->m_class != ACLCUR_REG) {
			err_puts("[carve1::address] no register for the operand");
			return c_lbl_err;
		}

		/** the storage we are about to hand out an address for must be
		 * materialised, or the OpVariable is dropped and the address
		 * references an id nothing defines. */
		aclexpr_ptr_taken(REG, CTX(rdwr_data));

		/** never cached, never twice: for a view this appends a second
		 * OpAccessChain for a single use. */
		ae2f_ifezerr(PTR = P_REG->m_d.m_reg.m_in.m_addr
				? P_REG->m_d.m_reg.m_in.m_addr(REG, CTX(rdwr_data))
				: 0) {
			err_puts("[carve1::address] no address for the operand");
			return c_lbl_err;
		}

		ae2f_ifezerr(TY_PTR = aclty_mk_ptr(
					P_REG->m_ty
					, (enum SpvStorageClass_)P_REG->m_d.m_reg.m_storage_class
					, CTX(rdwr_data))) {
			err_puts("[carve1::address] failed to make the pointer type");
			return c_lbl_err;
		}

		/** the address must be handed on as a typed SSA value, not as a
		 * bare id: a store into a pointer variable type-checks the value
		 * against the register, and aclcur_get_load rejects a register id
		 * (no VALUE flag). OpCopyObject is the cheapest way to get a real
		 * VALUE node of pointer type out of an address — and its operand is
		 * an address, so it goes through the raw-operand form. */
		ae2f_ifezerr(*rdwr_prv_idval = aclcur_cfg_op1r(
					0
					, SpvOpCopyObject
					, TY_PTR
					, PTR
					, CTX(rdwr_data)
					)) {
			err_puts("[carve1::address] failed to make the pointer value");
			return c_lbl_err;
		}

		*rdwr_prv_extra = rdwr_lst_extra[0];
		rdwr_prv_extra->m_type_original = TY_PTR;
		rdwr_prv_extra->m_mask_literal = 0;
	}

	return c_lbl_success;
}
