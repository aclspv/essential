#ifndef	aclexpr_visit_op2_bitwise_h
#define	aclexpr_visit_op2_bitwise_h

#include <aclexpr/visit/api.h>
#include <aclexpr/op.h>

#include <util/scale.h>
#include <util/wrdemit.h>

#include <ae2f/cc/memset.h>

ae2f_inline_f static aclexpr_visit_lbl_t aclexpr_visit_op2_bool0(
		const CXCursor				c_cur,
		const h_aclspv_ctx_t			h_ctx, 
		x_aclspv_vec* ae2f_restrict const	h_cmdscale,
		const aclexpr_visit_lbl_t		c_lbl_success,
		const aclexpr_visit_lbl_t		c_lbl_fail,
		const aclexpr_visit_lbl_t		c_lbl_pass
		)
{
	enum ACLEXPR_OP_	OPCODE;
	uint_leastenum_t	BOK	= (uint_leastenum_t)clang_getCursorBinaryOperatorKind(c_cur);

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

	switch(BOK) {
		default:
			return c_lbl_pass;
		case CXBinaryOperator_And:
			OPCODE = ACLEXPR_OP2_AND;
			break;
		case CXBinaryOperator_AndAssign:
			OPCODE = ACLEXPR_OP2_AND_A;
			break;
		case CXBinaryOperator_LAnd:
			OPCODE = ACLEXPR_OP2_AND_L;
			break;

		case CXBinaryOperator_Or:
			OPCODE = ACLEXPR_OP2_OR;
			break;
		case CXBinaryOperator_OrAssign:
			OPCODE = ACLEXPR_OP2_OR_A;
			break;
		case CXBinaryOperator_LOr:
			OPCODE = ACLEXPR_OP2_OR_L;
			break;

		case CXBinaryOperator_Xor:
			OPCODE = ACLEXPR_OP2_XOR;
			break;
		case CXBinaryOperator_XorAssign:
			OPCODE = ACLEXPR_OP2_XOR_A;
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


		ae2fcc_memset(&VEC[ACLEXPR_OPBINCFG_END + 2], 0, sizeof(aclexpr_opbincfg_extra) << 1);

		VEC[ACLEXPR_OPBINCFG_NUMOPRND] = 2;				/** number of operands */
		VEC[ACLEXPR_OPBINCFG_SPECCONSTANTOP] = aclspv_opcode_to_wrd(SpvOpSpecConstantOp, 5);
		VEC[ACLEXPR_OPBINCFG_OPCODE] = (aclspv_wrd_t) OPCODE;
	}

	return c_lbl_success;
}

#endif
