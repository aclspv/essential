#ifndef	aclexpr_visit_op1_deref_h
#define	aclexpr_visit_op1_deref_h

#include "../../op.h"
#include "../api.h"

#include <util/constant.h>
#include <util/scale.h>
#include <util/wrdemit.h>

#include <aclty.h>
#include <aclmem.h>

/**
 * address-of (`&x`) and dereference (`*p`) — both unary operators, one
 * operand (the lvalue / the pointer), one extra slot.
 *
 * neither emits an instruction at this level: `&x` yields the operand's
 * address and `*p` yields a load from the operand's pointer value, both
 * decided at carve time where the operand register node is reachable.
 * RESTYID is therefore left zero — a pointer type cannot be built without
 * the operand's storage class.
 *
 * @see ./../../../aclexpr/carve/op1/deref.c
 * */
ae2f_inline_f static aclexpr_visit_lbl_t aclexpr_visit_op1_deref(
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
	aclspv_wrd_t* ae2f_restrict	VEC;
	aclspv_wrd_t			OPCODE;

	ae2fpat_assertunless(h_ctx) {
		return c_lbl_fail;
	}

	unless(CXCursor_UnaryOperator == c_cur.kind)
		return c_lbl_pass;

	switch((uint_leastenum_t)clang_getCursorUnaryOperatorKind(c_cur)) {
		default:
			return c_lbl_pass;

		case CXUnaryOperator_AddrOf:
			OPCODE = ACLEXPR_OP1_ADDRESS;
			break;

		case CXUnaryOperator_Deref:
			OPCODE = ACLEXPR_OP1_DEREF;
			break;
	}

	/** assignment-target lvalue (`*o = x`): the assign stage pre-seeded
	 * STATE0_VAL_NO_NEED into this operand's extra slot (the same slot
	 * OP0_VAL reads for a plain variable lvalue). no read cascade, no
	 * scale: the deref-write carve re-derives the pointer from this
	 * deref cursor (extra[0]). */
	if(rdwr_extra->m_state0
			& (aclspv_wrd_t)ACLEXPR_OPCFGX_STATE0_VAL_NO_NEED) {
		dbg_puts("[visit1::deref] lvalue target: read skipped");
		rdwr_extra->m_mask_literal = 0;
		return c_lbl_collect;
	}

	NEW_SCALE = get_last_scale_from_vec(h_cmdscale);
	NEW_SCALE = mk_scale_from_vec(
			h_cmdscale
			, 
			aclutil_count_to_sz(ACLEXPR_OPBINCFG_END + 1)
			+ (sizeof(aclexpr_opbincfg_extra) * 1)
			);
	ae2f_expected_but_else(NEW_SCALE)
		return c_lbl_fail;

	VEC = get_buf_from_scale2(aclspv_wrd_t, h_cmdscale, *NEW_SCALE);

	ae2f_expected_but_else(VEC)
		return c_lbl_fail;

	VEC[ACLEXPR_OPBINCFG_NUMOPRND]	= 1;
	VEC[ACLEXPR_OPBINCFG_OPCODE]	= OPCODE;
	/** resolved at carve time: the pointer type needs the operand
	 * register's storage class, unreachable from the visit lane. */
	VEC[ACLEXPR_OPBINCFG_RESTYID]	= 0;
	VEC[ACLEXPR_OPBINCFG_END]	= 0;

	aclmemzero(&VEC[ACLEXPR_OPBINCFG_END + 1], sizeof(aclexpr_opbincfg_extra));

	return c_lbl_success;
}

#endif
