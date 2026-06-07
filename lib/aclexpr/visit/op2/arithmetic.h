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

#include <ae2f/cc/memset.h>

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
			IS_ASSIGN = ACLEXPR_OPCFGX_STATE0_ASSIGN; ae2f_fallthrough;
		case CXBinaryOperator_Add:
			OPCODE = ACLEXPR_OP2_ARITHMETIC_ADD;
			break;

		case CXBinaryOperator_SubAssign:
			IS_ASSIGN = ACLEXPR_OPCFGX_STATE0_ASSIGN; ae2f_fallthrough;
		case CXBinaryOperator_Sub:
			OPCODE = ACLEXPR_OP2_ARITHMETIC_SUB;
			break;

		case CXBinaryOperator_MulAssign:
			IS_ASSIGN = ACLEXPR_OPCFGX_STATE0_ASSIGN; ae2f_fallthrough;
		case CXBinaryOperator_Mul:
			OPCODE = ACLEXPR_OP2_ARITHMETIC_MUL;
			break;

		case CXBinaryOperator_DivAssign:
			IS_ASSIGN = ACLEXPR_OPCFGX_STATE0_ASSIGN; ae2f_fallthrough;
		case CXBinaryOperator_Div:
			OPCODE = ACLEXPR_OP2_ARITHMETIC_DIV;
			break;

		case CXBinaryOperator_RemAssign:
			IS_ASSIGN = ACLEXPR_OPCFGX_STATE0_ASSIGN; ae2f_fallthrough;
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


		VEC[ACLEXPR_OPBINCFG_NUMOPRND] = 2;				/** number of operands */
		VEC[ACLEXPR_OPBINCFG_SPECCONSTANTOP] = opcode_to_wrd(SpvOpSpecConstantOp, 5);
		VEC[ACLEXPR_OPBINCFG_OPCODE] = (aclspv_wrd_t) OPCODE;

		ae2fcc_memset(&VEC[ACLEXPR_OPBINCFG_END + 2], 0, sizeof(aclexpr_opbincfg_extra) << 1);

		((aclexpr_opbincfg_extra* ae2f_restrict)(void* ae2f_restrict)(&VEC[(ACLEXPR_OPBINCFG_END + 2)]))->m_state0 = IS_ASSIGN;
		((aclexpr_opbincfg_extra* ae2f_restrict)(void* ae2f_restrict)(&VEC[(ACLEXPR_OPBINCFG_END + 2)]))->m_mask_literal = 3;
	}

	return c_lbl_success;
}

#endif
