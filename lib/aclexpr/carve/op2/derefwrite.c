#include "./derefwrite.h"

#include <aclexpr/carve/util.h>
#include <aclexpr/carve/ptr.h>

#include <aclty.h>
#include <aclcur/common.h>
#include <aclcur/load.h>
#include <aclexpr/curreg.h>

#include <aclutil/emitx.h>

/** `*p = x` — OpStore through the pointer of the deref lvalue.
 *
 * the pointer is re-derived from the lvalue cursor (extra[0].m_cursor)
 * rather than from a collected operand, exactly like the member-write carve:
 * the assign stage suppressed the read cascade for this operand.
 *
 * OpStore carries neither Result Type nor Result, so it fits none of the
 * ACLCUR_OPn node shapes — it is configured through aclcur_cfg_st and
 * emitted in the emit pass, which is what keeps it ordered after the
 * instructions its operands came from.
 * */
aclexpr_carve_lbl_t aclexpr_carve2_derefwrite(
		struct aclexpr_visit_data* const ae2f_restrict	rdwr_data,
		aclspv_wrd_t* const ae2f_restrict	rdwr_prv_idval,
		aclexpr_opbincfg_extra* const ae2f_restrict rdwr_prv_extra,
		ae2f_unused h_scale_t			h_lst_scale,
		aclspv_wrd_t* const ae2f_restrict	rdwr_lst_scale_buf,
		aclexpr_opbincfg_extra* ae2f_restrict	rdwr_lst_extra,
		aclutil_literal* ae2f_restrict		r_ltrl,
		aclexpr_carve_lbl_t			c_lbl_continue,
		aclexpr_carve_lbl_t			c_lbl_success,
		aclexpr_carve_lbl_t			c_lbl_err
		) {
	const CXCursor DREF = rdwr_lst_extra[0].m_cursor;
	const aclspv_id_t X = rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_END + 1];
	aclexpr_opnd_t OPND;
	aclspv_id_t REG;
	aclspv_id_t PTR;

	unless(rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_OPCODE] == (aclspv_wrd_t)ACLEXPR_OP2_DEREFWRITE)
		return c_lbl_continue;

	unless(clang_getCursorKind(DREF) == CXCursor_UnaryOperator) {
		err_puts("[carve2::derefwrite] lvalue is not a dereference\n");
		return c_lbl_err;
	}

	/** the lvalue short-circuited the child cascade, so extra[0] is still
	 * the dereference itself: take its operand explicitly. */
	OPND = aclexpr_opnd_first_child(DREF);

	if(!OPND.m_has) {
		err_puts("[carve2::derefwrite] dereference has no pointer operand\n");
		return c_lbl_err;
	}

	ae2f_ifezerr(REG = aclexpr_opnd_reg(OPND.m_c, CTX(rdwr_data)))
		REG = 0;

	if(!REG || !X) {
		err_puts("[carve2::derefwrite] no pointer register or no value\n");
		return c_lbl_err;
	}

	/** refuse a non-pointer outright. without this the OpStore went through
	 * whatever id the operand happened to carry, which produced a module that
	 * is structurally wrong rather than one that says what is missing. */
	if(!aclexpr_ptr_require(REG, CTX(rdwr_data))) {
		err_puts("[carve2::derefwrite] the deref operand is not a pointer");
		return c_lbl_err;
	}

	ae2f_ifezerr(PTR = aclexpr_ptrval(REG, CTX(rdwr_data))) {
		err_puts("[carve2::derefwrite] no pointer value\n");
		return c_lbl_err;
	}

	{
		/** an emitted node, not words appended here: the emit pass runs
		 * after evaluation, so anything appended during carve would land
		 * before the instructions this store's operands come from. */
		ae2f_ifezerr(aclcur_cfg_st(
					SpvOpStore
					, PTR
					, X
					, CTX(rdwr_data)
					)) {
			err_puts("[carve2::derefwrite] OpStore failed\n");
			return c_lbl_err;
		}
	}

	*rdwr_prv_idval = X;
	*rdwr_prv_extra = rdwr_lst_extra[1];
	rdwr_prv_extra->m_literal = *r_ltrl;

	return c_lbl_success;
}
