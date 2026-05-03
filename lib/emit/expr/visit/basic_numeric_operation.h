/** 
 * @file	basic_numeric_operation.h
 * @brief	`+, -, *, /, %`
 * */

#ifndef	aclexpr_visit_basic_numeric_operation_h
#define aclexpr_visit_basic_numeric_operation_h

#include <clang-c/CXString.h>
#include <clang-c/Index.h>
#include <util/scale.h>
#include <util/op.h>
#include <util/emitx.h>

#include "../../expr.h"
#include "ae2f/cc.h"
#include "util/log.h"

ae2f_inline static enum EMIT_EXPR_ s_basic_numeric_operation(
		const CXCursor				c_cur,
		const h_util_ctx_t			h_ctx, 
		x_aclspv_vec* ae2f_restrict const	h_cmdscale
		)
{
	enum UTIL_OPBIN2_	OPCODE;
	aclspv_bool_t		IS_ASSIGN = 0;

	ae2f_expected_but_else(h_ctx) {
		assert(0 && "emit_expr_bin_2::h_ctx is null");
		return EMIT_EXPR_FAILURE;
	}

#if	DBG
	{
		const CXString SPL = clang_getCursorSpelling(c_cur);
		const CXString KNDSPL = clang_getCursorKindSpelling(clang_getCursorKind(c_cur));
		const char* const OP = clang_getCString(SPL);
		const char* const OPKND = clang_getCString(KNDSPL);

		ae2f_expected_but_else(OP) {
			h_ctx->m_err = ACLSPV_COMPILE_ERR_CLANG;
			return EMIT_EXPR_FAILURE;
		}

		dbg_prefix(a);
		dbg_call((void), fprintf(stderr, "Binary Operator: %s\n", OP));
		dbg_call((void), fprintf(stderr, "Binary Operator Kind: %s\n", OPKND));

		clang_disposeString(SPL);
		clang_disposeString(KNDSPL);
	}
#endif

	switch((uint_leastenum_t)clang_getCursorKind(c_cur)) {
		default:
			return EMIT_EXPR_NOT_THE_CASE;

		case CXCursor_BinaryOperator:
		case CXCursor_CompoundAssignOperator:
			break;
	}

	switch((uint_leastenum_t)clang_getCursorBinaryOperatorKind(c_cur)) {
		default:
			return EMIT_EXPR_NOT_THE_CASE;

		case CXBinaryOperator_AddAssign:
			IS_ASSIGN |= 1; ae2f_fallthrough;
		case CXBinaryOperator_Add:
			OPCODE = UTIL_OPBIN2_ADD;
			break;

		case CXBinaryOperator_SubAssign:
			IS_ASSIGN |= 1; ae2f_fallthrough;
		case CXBinaryOperator_Sub:
			OPCODE = UTIL_OPBIN2_SUB;
			break;

		case CXBinaryOperator_MulAssign:
			IS_ASSIGN |= 1; ae2f_fallthrough;
		case CXBinaryOperator_Mul:
			OPCODE = UTIL_OPBIN2_MUL;
			break;

		case CXBinaryOperator_DivAssign:
			IS_ASSIGN |= 1; ae2f_fallthrough;
		case CXBinaryOperator_Div:
			OPCODE = UTIL_OPBIN2_DIV;
			break;

		case CXBinaryOperator_RemAssign:
			IS_ASSIGN |= 1; ae2f_fallthrough;
		case CXBinaryOperator_Rem:
			OPCODE = UTIL_OPBIN2_REM;
			break;
	}

	{
		aclspv_wrd_t* ae2f_restrict VEC;
		x_scale* ae2f_restrict	NEW_SCALE;

		NEW_SCALE = get_last_scale_from_vec(h_cmdscale);

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

		((util_opbincfg_extra* ae2f_restrict)(void* ae2f_restrict)(&VEC[(UTIL_OPBINCFG_END + 2)]))[0].m_is_assign = IS_ASSIGN;
		((util_opbincfg_extra* ae2f_restrict)(void* ae2f_restrict)(&VEC[(UTIL_OPBINCFG_END + 2)]))[1].m_is_assign = IS_ASSIGN;
	}

	return EMIT_EXPR_SUCCESS_LITERAL;
}

#endif
