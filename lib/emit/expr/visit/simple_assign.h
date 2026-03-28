/** 
 * @file simple_assign.h 
 * */

#ifndef aclexpr_visit_assign_simple_h
#define aclexpr_visit_assign_simple_h

#include <clang-c/Index.h>
#include <util/constant.h>
#include <util/scale.h>
#include <util/op.h>
#include <util/cxtp.h>
#include <util/wrdemit.h>

#include "../../expr.h"

ae2f_inline static enum EMIT_EXPR_ s_simple_assign(
		const CXCursor				c_cur,
		const h_util_ctx_t			h_ctx, 
		x_aclspv_vec* ae2f_restrict const	h_cmdscale
		)
{
	aclspv_wrd_t	OPCODE;
	switch((uint_leastenum_t)c_cur.kind) {
		default:
			return EMIT_EXPR_NOT_THE_CASE;

		case CXCursor_BinaryOperator:
			break;
	}

	switch((uint_leastenum_t)clang_getCursorBinaryOperatorKind(c_cur)) {
		default:
			return EMIT_EXPR_NOT_THE_CASE;

		case CXBinaryOperator_AddAssign:
			OPCODE = UTIL_OPBIN2_ADDASSIGN;
		case CXBinaryOperator_Add:
			OPCODE = UTIL_OPBIN2_ADD;
			break;

		case CXBinaryOperator_SubAssign:
			OPCODE = UTIL_OPBIN2_SUBASSIGN;

		case CXBinaryOperator_Sub:
			OPCODE = UTIL_OPBIN2_SUB;
			break;

		case CXBinaryOperator_Mul:
			OPCODE = UTIL_OPBIN2_MUL;
			break;

		case CXBinaryOperator_Div:
			OPCODE = UTIL_OPBIN2_DIV;
			break;

		case CXBinaryOperator_Rem:
			OPCODE = UTIL_OPBIN2_REM;
			break;
	}

	{
		aclspv_wrd_t* ae2f_restrict VEC;
		x_scale* ae2f_restrict	NEW_SCALE = get_last_scale_from_vec(h_cmdscale);

		NEW_SCALE = mk_scale_from_vec(
				h_cmdscale
				, 
				count_to_sz(UTIL_OPBINCFG_END + 2)
				+ (sizeof(util_opbincfg_extra) * 2)
				);
		ae2f_expected_but_else(NEW_SCALE)
			return EMIT_EXPR_FAILURE;

		VEC = get_buf_from_scale2(aclspv_wrd_t, h_cmdscale, *NEW_SCALE);

		ae2f_expected_but_else(VEC)
			return EMIT_EXPR_FAILURE;

		VEC[0] = 2;				/** number of operands */
		VEC[1] = UTIL_LITERAL_MASK_LITERAL;	/** is not constant? */
		VEC[2] = opcode_to_wrd(SpvOpSpecConstantOp, 5);
		VEC[3] = 0;				/** return type */
		VEC[4] = 0;				/** return id (not specified) */
		VEC[5] = OPCODE;
		VEC[6] = 0;
		VEC[7] = 0;
	}
}

#endif
