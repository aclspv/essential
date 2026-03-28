/**
 * @file	cast_numeric.h
 * @brief	int & float cast
 * */

#ifndef	aclexpr_visit_cast_numeric_h
#define	aclexpr_visit_cast_numeric_h

#include <util/constant.h>
#include <util/scale.h>
#include <util/op.h>
#include <util/cxtp.h>
#include <util/wrdemit.h>

#include "../../expr.h"

ae2f_inline static enum EMIT_EXPR_ s_cast_numeric(
		const CXCursor				c_cur,
		const h_util_ctx_t			h_ctx, 
		x_aclspv_vec* ae2f_restrict const	h_cmdscale
		)
{
	x_scale* ae2f_restrict	NEW_SCALE;
	enum UTIL_OPBIN1_	OPCODE;
	aclspv_wrd_t* ae2f_restrict VEC;

	ae2fpat_assertunless(h_ctx) {
		return EMIT_EXPR_FAILURE;
	}

	switch((uint_leastenum_t)c_cur.kind) {
		default:
			return EMIT_EXPR_NOT_THE_CASE;

			/** casts */
		case CXCursor_CStyleCastExpr:
		case CXCursor_CXXStaticCastExpr:
		case CXCursor_CXXDynamicCastExpr:
		case CXCursor_CXXReinterpretCastExpr:
		case CXCursor_CXXConstCastExpr:
		case CXCursor_CXXFunctionalCastExpr:
			OPCODE = UTIL_OPBIN1_EXPLICIT_CAST;
	}

	{
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

		VEC[0] = 1;				/** number of operands */
		VEC[1] = UTIL_LITERAL_MASK_LITERAL;	/** is not constant? */
		VEC[2] = opcode_to_wrd(SpvOpSpecConstantOp, 4); 
		VEC[3] = 0;				/** c_result_type_id */
		VEC[4] = 0;				/** return id (not specified) */
		VEC[5] = OPCODE;
		VEC[6] = 0;				/** operand 0 */
	}

	switch((uint_leastenum_t)c_cur.kind) {
		default:
			break;

		case CXCursor_CStyleCastExpr:
		case CXCursor_CXXStaticCastExpr:
		case CXCursor_CXXDynamicCastExpr:
		case CXCursor_CXXReinterpretCastExpr:
		case CXCursor_CXXConstCastExpr:
		case CXCursor_CXXFunctionalCastExpr:
			VEC[UTIL_OPBINCFG_RESTYID] = aclutil_mk_cxtp_by_cursor(
					c_cur
					, clang_getCursorType(c_cur)
					, h_ctx
					);	/** new type */
	}

	return EMIT_EXPR_SUCCESS_LITERAL;
}

#endif
