/**
 * @file	shift.h
 * @brief	bitshift operators: `<<, >>, <<=, >>=`
 *
 *	shifts produce a plain common integer result type (no bool swap).
 * */
 
#ifndef	aclexpr_visit_op2_shift_h
#define	aclexpr_visit_op2_shift_h

#include <util/scale.h>
#include <util/emitx.h>
#include <util/log.h>

#include <aclexpr.h>
#include <aclexpr/visit/api.h>
#include <aclexpr/op.h>

#include <aclmem.h>

ae2f_inline_f static aclexpr_visit_lbl_t aclexpr_visit_op2_shift(
		const CXCursor				c_cur,
		const h_aclspv_ctx_t			h_ctx, 
		x_aclspv_vec* ae2f_restrict const	h_cmdscale,
		const aclexpr_visit_lbl_t		c_lbl_success,
		const aclexpr_visit_lbl_t		c_lbl_fail,
		const aclexpr_visit_lbl_t		c_lbl_pass
		)
{
	enum ACLEXPR_OP_	OPCODE;

	ae2f_expected_but_else(h_ctx) {
		return c_lbl_fail;
	}

	switch((uint_leastenum_t)clang_getCursorKind(c_cur)) {
		default:
			return c_lbl_pass;

		case CXCursor_BinaryOperator:
		case CXCursor_CompoundAssignOperator:
			break;
	}

	switch((uint_leastenum_t)clang_getCursorBinaryOperatorKind(c_cur)) {
		default:
			return c_lbl_pass;

		case CXBinaryOperator_Shl:
			OPCODE = ACLEXPR_OP2_SHL;
			break;
		case CXBinaryOperator_ShlAssign:
			OPCODE = ACLEXPR_OP2_SHLASS;
			break;
		case CXBinaryOperator_Shr:
			OPCODE = ACLEXPR_OP2_SHR;
			break;
		case CXBinaryOperator_ShrAssign:
			OPCODE = ACLEXPR_OP2_SHRASS;
			break;
	}

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
		ae2f_expected_but_else(NEW_SCALE)
			return c_lbl_fail;

		VEC = get_buf_from_scale2(aclspv_wrd_t, h_cmdscale, *NEW_SCALE);

		ae2f_expected_but_else(VEC)
			return c_lbl_fail;


		VEC[ACLEXPR_OPBINCFG_NUMOPRND] = 2;				/** number of operands */
		VEC[ACLEXPR_OPBINCFG_SPECCONSTANTOP] = aclspv_opcode_to_wrd(SpvOpSpecConstantOp, 5);
		VEC[ACLEXPR_OPBINCFG_OPCODE] = (aclspv_wrd_t) OPCODE;

		aclmemzero(&VEC[ACLEXPR_OPBINCFG_END + 2], sizeof(aclexpr_opbincfg_extra) << 1);

		((aclexpr_opbincfg_extra* ae2f_restrict)(void* ae2f_restrict)(&VEC[(ACLEXPR_OPBINCFG_END + 2)]))->m_mask_literal = 3;
	}

	return c_lbl_success;
}

#endif

