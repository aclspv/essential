#ifndef	aclexpr_visit_op1_subscript_h
#define	aclexpr_visit_op1_subscript_h

#include "../../op.h"
#include "../api.h"

#include <util/constant.h>
#include <util/scale.h>
#include <util/wrdemit.h>

#include <aclty.h>
#include <aclmem.h>

/**
 * array subscript (`a[i]`).
 *
 * two operands — the array and the index — so the scale declares NUMOPRND 2
 * and both children cascade normally. the index may be dynamic: it is an
 * ordinary expression, evaluated to a value id before the carve builds the
 * view around it.
 *
 * RESTYID stays zero: the element type comes from the array's own aclty at
 * carve time, and the view's pointer type needs the array's storage class.
 *
 * @see ../../../../aclcur/reg/view.c for the chain the carve builds.
 * */
ae2f_inline_f static aclexpr_visit_lbl_t aclexpr_visit_op1_subscript(
		const CXCursor				c_cur,
		const h_aclspv_ctx_t			h_ctx,
		x_aclspv_vec* ae2f_restrict const	h_cmdscale,
		aclexpr_opbincfg_extra* ae2f_restrict	rdwr_extra,
		aclexpr_visit_lbl_t			c_lbl_success,
		aclexpr_visit_lbl_t			c_lbl_fail,
		aclexpr_visit_lbl_t			c_lbl_pass,
		aclexpr_visit_lbl_t			c_lbl_collect
		)
{
	x_scale* ae2f_restrict		NEW_SCALE;
	x_scale* ae2f_restrict		PRV_SCALE;
	aclspv_wrd_t* ae2f_restrict	VEC;
	aclspv_wrd_t* ae2f_restrict	PRV_BUF;
	aclexpr_opbincfg_extra* ae2f_restrict SCL_EXTRA;

	ae2fpat_assertunless(h_ctx) {
		return c_lbl_fail;
	}

	unless(CXCursor_ArraySubscriptExpr == c_cur.kind)
		return c_lbl_pass;

	/** assignment-target lvalue (`a[i] = v`): the assign stage pre-seeded
	 * STATE0_VAL_NO_NEED, so no read cascade and no scale here — the
	 * subscript-write carve re-derives array and index from this cursor. */
	if(rdwr_extra->m_state0
			& (aclspv_wrd_t)ACLEXPR_OPCFGX_STATE0_VAL_NO_NEED) {
		dbg_puts("[visit1::subscript] lvalue target: read skipped");
		rdwr_extra->m_mask_literal = 0;
		return c_lbl_collect;
	}

	NEW_SCALE = mk_scale_from_vec(
			h_cmdscale
			,
			aclutil_count_to_sz(ACLEXPR_OPBINCFG_END + 2)
			+ (sizeof(aclexpr_opbincfg_extra) * 2)
			);
	ae2f_expected_but_else(NEW_SCALE)
		return c_lbl_fail;

	VEC = get_buf_from_scale2(aclspv_wrd_t, h_cmdscale, *NEW_SCALE);

	ae2f_expected_but_else(VEC)
		return c_lbl_fail;

	VEC[ACLEXPR_OPBINCFG_NUMOPRND]	= 2;
	VEC[ACLEXPR_OPBINCFG_OPCODE]	= ACLEXPR_OP2_SUBSCRIPT;
	VEC[ACLEXPR_OPBINCFG_RESTYID]	= 0;
	VEC[ACLEXPR_OPBINCFG_END]	= 0;
	VEC[ACLEXPR_OPBINCFG_END + 1]	= 0;

	aclmemzero(&VEC[ACLEXPR_OPBINCFG_END + 2], sizeof(aclexpr_opbincfg_extra) * 2);

	/** `&a[i]`: the address-of wants the element's ADDRESS, not its value.
	 *
	 * the cascade below is exactly what `a[i]` needs — it is the only way a
	 * dynamic index like `idx - 1` gets evaluated to a value id before the
	 * carve — so it runs here unchanged and the subscript carve learns from
	 * this flag that its OpLoad must become an OpCopyObject instead. the flag
	 * rides extra[0].m_state0 of THIS scale: the children only write
	 * m_cursor and m_mask_literal on the way down, and the wholesale
	 * PRV_EXTRA write-back happens in visit.c only once every operand has been
	 * collected, i.e. after this carve has already run.
	 *
	 * the parent is reached through NEW_SCALE's m_prv and resolved HERE,
	 * after the push: mk_scale_from_vec reallocs the scale arena, so any
	 * parent pointer taken before it dangles. */
	PRV_SCALE = get_prv_from_scale(h_cmdscale, *NEW_SCALE);
	PRV_BUF = PRV_SCALE
			? get_buf_from_scale2(aclspv_wrd_t, h_cmdscale, *PRV_SCALE)
			: 0;

	if(PRV_BUF
			&& PRV_BUF[ACLEXPR_OPBINCFG_OPCODE]
					== (aclspv_wrd_t)ACLEXPR_OP1_ADDRESS) {
		dbg_puts("[visit1::subscript] address-of subscript: address instead of value");
		SCL_EXTRA = (void* ae2f_restrict)&VEC[ACLEXPR_OPBINCFG_END + 2];
		SCL_EXTRA->m_state0
			|= (aclspv_wrd_t)ACLEXPR_OPCFGX_STATE0_ADDR_OF_ELM;
	}

	return c_lbl_success;
}

#endif
