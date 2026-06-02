/** 
 * @file	basic_numeric_operation.h
 * @brief	`+, -, *, /, %`
 * */

#ifndef	aclexpr_visit_basic_numeric_operation_h
#define aclexpr_visit_basic_numeric_operation_h

#include <util/scale.h>
#include <util/emitx.h>
#include <util/log.h>

#include <aclexpr.h>
#include <aclexpr/visit/api.h>
#include <aclexpr/op.h>

ae2f_inline_f static aclexpr_visit_lbl_t aclexpr_visit_op2_arithmetic(
		const CXCursor				c_cur,
		const h_aclspv_ctx_t			h_ctx, 
		x_aclspv_vec* ae2f_restrict const	h_cmdscale,
		const aclexpr_visit_lbl_t		c_lbl_success,
		const aclexpr_visit_lbl_t		c_lbl_fail,
		const aclexpr_visit_lbl_t		c_lbl_pass
		)
{
	enum ACLEXPR_OP_	OPCODE;
	aclspv_bool_t		IS_ASSIGN = 0;

	ae2f_expected_but_else(h_ctx) {
		return c_lbl_fail;
	}

#if	DBG
	{
		const CXString SPL = clang_getCursorSpelling(c_cur);
		const CXString KNDSPL = clang_getCursorKindSpelling(clang_getCursorKind(c_cur));
		const char* const OP = clang_getCString(SPL);
		const char* const OPKND = clang_getCString(KNDSPL);

		ae2f_expected_but_else(OP) {
			h_ctx->m_err = ACLSPV_COMPILE_ERR_CLANG;
			return c_lbl_fail;
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
			return c_lbl_pass;

		case CXCursor_BinaryOperator:
		case CXCursor_CompoundAssignOperator:
			break;
	}

	switch((uint_leastenum_t)clang_getCursorBinaryOperatorKind(c_cur)) {
		default:
			return c_lbl_pass;

		case CXBinaryOperator_AddAssign:
			IS_ASSIGN |= 1; ae2f_fallthrough;
		case CXBinaryOperator_Add:
			OPCODE = ACLEXPR_OP2_ARITHMETIC_ADD;
			break;

		case CXBinaryOperator_SubAssign:
			IS_ASSIGN |= 1; ae2f_fallthrough;
		case CXBinaryOperator_Sub:
			OPCODE = ACLEXPR_OP2_ARITHMETIC_SUB;
			break;

		case CXBinaryOperator_MulAssign:
			IS_ASSIGN |= 1; ae2f_fallthrough;
		case CXBinaryOperator_Mul:
			OPCODE = ACLEXPR_OP2_ARITHMETIC_MUL;
			break;

		case CXBinaryOperator_DivAssign:
			IS_ASSIGN |= 1; ae2f_fallthrough;
		case CXBinaryOperator_Div:
			OPCODE = ACLEXPR_OP2_ARITHMETIC_DIV;
			break;

		case CXBinaryOperator_RemAssign:
			IS_ASSIGN |= 1; ae2f_fallthrough;
		case CXBinaryOperator_Rem:
			OPCODE = ACLEXPR_OP2_ARITHMETIC_REM;
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

		VEC[0] = 2;				/** number of operands */
		VEC[1] = ACLUTIL_LITERAL_MASK_LITERAL;	/** is not constant? */
		VEC[2] = opcode_to_wrd(SpvOpSpecConstantOp, 5);
		VEC[3] = 0;				/** return type */
		VEC[4] = 0;				/** return id (not specified) */
		VEC[5] = (aclspv_wrd_t) OPCODE;
		VEC[6] = 0;
		VEC[7] = 0;

		((aclexpr_opbincfg_extra* ae2f_restrict)(void* ae2f_restrict)(&VEC[(ACLEXPR_OPBINCFG_END + 2)]))[0].m_is_assign = IS_ASSIGN;
		((aclexpr_opbincfg_extra* ae2f_restrict)(void* ae2f_restrict)(&VEC[(ACLEXPR_OPBINCFG_END + 2)]))[1].m_is_assign = IS_ASSIGN;
	}

	return c_lbl_success;
}

#endif
