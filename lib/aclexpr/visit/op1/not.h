#ifndef	aclexpr_visit_op1_not_h
#define	aclexpr_visit_op1_not_h

#include "../../op.h"
#include "../api.h"

#include <util/scale.h>
#include <aclutil/wrdemit.h>

#include <aclty.h>
#include <aclexpr.h>
#include <aclmem.h>

/** push a one-operand scale for a unary operator. */
static aclexpr_visit_lbl_t s_visit_op1_push(
		x_aclspv_vec* ae2f_restrict const	h_cmdscale,
		aclspv_wrd_t				c_opcode,
		aclspv_id_t				c_res_ty,
		aclexpr_visit_lbl_t			c_lbl_success,
		aclexpr_visit_lbl_t			c_lbl_fail
		) {
	x_scale* ae2f_restrict		NEW_SCALE;
	aclspv_wrd_t* ae2f_restrict	VEC;

	NEW_SCALE = mk_scale_from_vec(
			h_cmdscale
			, aclutil_count_to_sz(ACLEXPR_OPBINCFG_END + 1)
			+ sizeof(aclexpr_opbincfg_extra)
			);
	ae2f_ifezerr(NEW_SCALE) return c_lbl_fail;

	VEC = get_buf_from_scale2(aclspv_wrd_t, h_cmdscale, *NEW_SCALE);
	ae2f_ifezerr(VEC) return c_lbl_fail;

	VEC[ACLEXPR_OPBINCFG_NUMOPRND]	= 1;
	VEC[ACLEXPR_OPBINCFG_OPCODE]	= c_opcode;
	VEC[ACLEXPR_OPBINCFG_RESTYID]	= c_res_ty;
	aclmemzero(&VEC[ACLEXPR_OPBINCFG_END + 1], sizeof(aclexpr_opbincfg_extra));

	return c_lbl_success;
}

/** `!x` — scalar: OpLogicalNot over a bool-cast operand; vector: per-element
 * (x == 0) ? -1 : 0, matching Clang/OpenCL. */
static aclexpr_visit_lbl_t aclexpr_visit_op1_not(
		const CXCursor				c_cur,
		const h_aclspv_ctx_t			h_ctx,
		x_aclspv_vec* ae2f_restrict const	h_cmdscale,
		aclexpr_visit_lbl_t			c_lbl_success,
		aclexpr_visit_lbl_t			c_lbl_fail,
		aclexpr_visit_lbl_t			c_lbl_pass
		) {
	aclspv_id_t TY;

	if(c_cur.kind != CXCursor_UnaryOperator)
		return c_lbl_pass;

	if(clang_getCursorUnaryOperatorKind(c_cur) != CXUnaryOperator_LNot)
		return c_lbl_pass;

	/** the declared result type is int for `!int`, int4 for `!int4`, and
	 * int4 for `!float4`; the carve uses its shape to pick the lane. */
	TY = aclutil_mk_cxtp_by_type2(
			clang_getCanonicalType(clang_getCursorType(c_cur))
			, h_ctx);
	ae2f_ifezerr(TY) return c_lbl_fail;

	return s_visit_op1_push(
			h_cmdscale
			, (aclspv_wrd_t)ACLEXPR_OP1_NOT
			, TY
			, c_lbl_success, c_lbl_fail);
}

/** `~x` — OpNot on the (already promoted) result type. */
static aclexpr_visit_lbl_t aclexpr_visit_op1_bitnot(
		const CXCursor				c_cur,
		const h_aclspv_ctx_t			h_ctx,
		x_aclspv_vec* ae2f_restrict const	h_cmdscale,
		aclexpr_visit_lbl_t			c_lbl_success,
		aclexpr_visit_lbl_t			c_lbl_fail,
		aclexpr_visit_lbl_t			c_lbl_pass
		) {
	aclspv_id_t TY;

	if(c_cur.kind != CXCursor_UnaryOperator)
		return c_lbl_pass;

	if(clang_getCursorUnaryOperatorKind(c_cur) != CXUnaryOperator_Not)
		return c_lbl_pass;

	/** integer promotion already happened: the cursor's type is the
	 * promoted type the result must take. */
	TY = aclutil_mk_cxtp_by_type2(
			clang_getCanonicalType(clang_getCursorType(c_cur))
			, h_ctx);
	ae2f_ifezerr(TY) return c_lbl_fail;

	return s_visit_op1_push(
			h_cmdscale
			, (aclspv_wrd_t)ACLEXPR_OP1_BITNOT
			, TY
			, c_lbl_success, c_lbl_fail);
}

#endif
