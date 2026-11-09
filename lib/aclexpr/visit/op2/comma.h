#ifndef	aclexpr_visit_op2_comma_h
#define	aclexpr_visit_op2_comma_h

#include "../../op.h"
#include "../api.h"

#include <util/scale.h>
#include <aclutil/wrdemit.h>

#include <aclty.h>
#include <aclexpr.h>
#include <aclmem.h>

/** `a, b` — evaluate both, the value is `b`.
 *
 * a two-operand scale, so the generic cascade evaluates the left for its
 * side effects before the right. the carve emits nothing of its own and
 * returns the right operand's value.
 * */
static aclexpr_visit_lbl_t aclexpr_visit_op2_comma(
		const CXCursor				c_cur,
		const h_aclspv_ctx_t			h_ctx,
		x_aclspv_vec* ae2f_restrict const	h_cmdscale,
		aclexpr_visit_lbl_t			c_lbl_success,
		aclexpr_visit_lbl_t			c_lbl_fail,
		aclexpr_visit_lbl_t			c_lbl_pass
		) {
	x_scale* ae2f_restrict		NEW_SCALE;
	aclspv_wrd_t* ae2f_restrict	VEC;
	aclspv_id_t			TY;

	if(c_cur.kind != CXCursor_BinaryOperator)
		return c_lbl_pass;

	if(clang_getCursorBinaryOperatorKind(c_cur) != CXBinaryOperator_Comma)
		return c_lbl_pass;

	TY = aclutil_mk_cxtp_by_type2(
			clang_getCanonicalType(clang_getCursorType(c_cur))
			, h_ctx);
	ae2f_ifezerr(TY) return c_lbl_fail;

	NEW_SCALE = mk_scale_from_vec(
			h_cmdscale
			, aclutil_count_to_sz(ACLEXPR_OPBINCFG_END + 2)
			+ (sizeof(aclexpr_opbincfg_extra) * 2)
			);
	ae2f_ifezerr(NEW_SCALE) return c_lbl_fail;

	VEC = get_buf_from_scale2(aclspv_wrd_t, h_cmdscale, *NEW_SCALE);
	ae2f_ifezerr(VEC) return c_lbl_fail;

	VEC[ACLEXPR_OPBINCFG_NUMOPRND]	= 2;
	VEC[ACLEXPR_OPBINCFG_OPCODE]	= (aclspv_wrd_t)ACLEXPR_OP2_COMMA;
	VEC[ACLEXPR_OPBINCFG_RESTYID]	= TY;
	aclmemzero(&VEC[ACLEXPR_OPBINCFG_END + 2], sizeof(aclexpr_opbincfg_extra) * 2);

	return c_lbl_success;
}

#endif
