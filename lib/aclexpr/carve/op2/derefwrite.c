#include "./derefwrite.h"

#include <aclexpr/carve/util.h>
#include <aclexpr/carve/ptr.h>
#include <aclexpr/util/unwrap.h>

#include <aclty.h>
#include <aclcur/common.h>
#include <aclcur/load.h>
#include <aclcur/store.h>
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
aclspv_bool_t aclexpr_carve2_derefwrite(
		struct aclexpr_visit_data	ae2fcc_arg_sarr_restrict(rdwr_data, 1),
		aclspv_wrd_t			ae2fcc_arg_sarr_restrict(rdwr_prv_idval, 1),
		aclexpr_opbincfg_extra		ae2fcc_arg_sarr_restrict(rdwr_prv_extra, 1),
		aclspv_wrd_t			ae2fcc_arg_sarr_restrict(rdwr_lst_scale_buf, ACLEXPR_OPBINCFG_END + 2),
		aclexpr_opbincfg_extra		ae2fcc_arg_sarr_restrict(rdwr_lst_extra, 2),
		aclutil_literal			ae2fcc_arg_sarr_restrict(r_ltrl, 1)
		) {
	const CXCursor DREF = aclexpr_expr_unwrap(rdwr_lst_extra[0].m_cursor);
	const aclspv_id_t X = rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_END + 1];
	aclexpr_opnd_t OPND;
	aclspv_id_t REG;
	aclspv_id_t PTR;
	aclspv_id_t GHOST;

	unless(clang_getCursorKind(DREF) == CXCursor_UnaryOperator) {
		err_puts("[carve2::derefwrite] lvalue is not a dereference\n");
		return ACLEXPR_CARVE_NO;
	}

	/** the lvalue short-circuited the child cascade, so extra[0] is still
	 * the dereference itself: take its operand explicitly. */
	OPND = aclexpr_opnd_first_child(DREF);

	if(!OPND.m_has) {
		err_puts("[carve2::derefwrite] dereference has no pointer operand\n");
		return ACLEXPR_CARVE_NO;
	}

	ae2f_ifezerr(REG = aclexpr_opnd_reg(aclexpr_expr_unwrap(OPND.m_c), CTX(rdwr_data)))
		REG = 0;

	if(!REG || !X) {
		err_puts("[carve2::derefwrite] no pointer register or no value\n");
		return ACLEXPR_CARVE_NO;
	}

	/** refuse a non-pointer outright. without this the OpStore went through
	 * whatever id the operand happened to carry, which produced a module that
	 * is structurally wrong rather than one that says what is missing. */
	if(!aclexpr_ptr_require(REG, CTX(rdwr_data))) {
		err_puts("[carve2::derefwrite] the deref operand is not a pointer");
		return ACLEXPR_CARVE_NO;
	}

	ae2f_ifezerr(PTR = aclexpr_ptrval(REG, CTX(rdwr_data))) {
		err_puts("[carve2::derefwrite] no pointer value\n");
		return ACLEXPR_CARVE_NO;
	}

	/** D6/D7: store through the ghost the read builds for the same pointer
	 * (aclcur_reg_ghost dedups on the held pointer value), so a plain deref
	 * store untracks the tracked value exactly the way any view store
	 * does. an emitted node, not words appended here: the emit pass runs
	 * after evaluation, so anything appended during carve would land
	 * before the instructions this store's operands come from. */
	ae2f_ifezerr(GHOST = aclcur_reg_ghost(PTR, CTX(rdwr_data))) {
		err_puts("[carve2::derefwrite] no ghost for the deref operand\n");
		return ACLEXPR_CARVE_NO;
	}

	ae2f_ifezerr(aclcur_store_reg(GHOST, X, CTX(rdwr_data))) {
		err_puts("[carve2::derefwrite] OpStore failed\n");
		return ACLEXPR_CARVE_NO;
	}

	*rdwr_prv_idval = X;
	*rdwr_prv_extra = rdwr_lst_extra[1];
	rdwr_prv_extra->m_literal = *r_ltrl;

	return ACLEXPR_CARVE_OK;
}
