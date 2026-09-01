/**
 * @file	cast_numeric.h
 * @brief	int & float cast
 * */

#ifndef	aclexpr_visit_cast_numeric_h
#define	aclexpr_visit_cast_numeric_h

#include "../../op.h"
#include "../api.h"

#include <util/constant.h>
#include <util/scale.h>
#include <util/wrdemit.h>

#include <aclty.h>
#include <aclexpr.h>

#include <ae2f/cc/memset.h>

ae2f_inline_f static aclexpr_visit_lbl_t aclexpr_visit_op1_arithmetic_cast(
		const CXCursor				c_cur,
		const h_aclspv_ctx_t			h_ctx, 
		x_aclspv_vec* ae2f_restrict const	h_cmdscale,
		aclexpr_visit_lbl_t			c_lbl_success,
		aclexpr_visit_lbl_t			c_lbl_fail,
		aclexpr_visit_lbl_t			c_lbl_pass
		)
{
	x_scale* ae2f_restrict	NEW_SCALE;
	enum ACLEXPR_OP_	OPCODE;
	aclspv_wrd_t* ae2f_restrict VEC;

	ae2fpat_assertunless(h_ctx) {
		return c_lbl_fail;
	}

	switch((uint_leastenum_t)c_cur.kind) {
		default:
			return c_lbl_pass;

			/** casts */
		case CXCursor_CStyleCastExpr:
		case CXCursor_CXXStaticCastExpr:
		case CXCursor_CXXDynamicCastExpr:
		case CXCursor_CXXReinterpretCastExpr:
		case CXCursor_CXXConstCastExpr:
		case CXCursor_CXXFunctionalCastExpr:
			OPCODE = ACLEXPR_OP1_EXPLICIT_CAST;
			NEW_SCALE = get_last_scale_from_vec(h_cmdscale);
			NEW_SCALE = mk_scale_from_vec(
					h_cmdscale
					, 
					aclutil_count_to_sz(ACLEXPR_OPBINCFG_END + 1)
					+ (sizeof(aclexpr_opbincfg_extra))
					);
			ae2f_expected_but_else(NEW_SCALE)
				return c_lbl_fail;

			VEC = get_buf_from_scale2(aclspv_wrd_t, h_cmdscale, *NEW_SCALE);

			ae2f_expected_but_else(VEC)
				return c_lbl_fail;

			VEC[ACLEXPR_OPBINCFG_NUMOPRND]	= 1;				/** number of operands */
			VEC[ACLEXPR_OPBINCFG_OPCODE]	= (aclspv_wrd_t) OPCODE;

			ae2fcc_memset(&VEC[ACLEXPR_OPBINCFG_END + 1], 0, sizeof(aclexpr_opbincfg_extra));

			ae2f_ifezerr(VEC[ACLEXPR_OPBINCFG_RESTYID] = aclty_mk_by_cursor(
						c_cur
						, clang_getCursorType(c_cur)
						, h_ctx
						)) {
				err_puts("[s_cast_numeric] failed aclutil_mk_cxtp_by_cursor");
				h_ctx->m_err = ACLSPV_COMPILE_ARITHMETIC_CAST_TYPE_IS_UNDEFINED_NEW;
				return c_lbl_fail;
			}

			ae2f_ifezerr_strict(aclid_is_arithmetic(VEC[ACLEXPR_OPBINCFG_RESTYID], h_ctx)) {
				err_prefix(a);
				err_call((void)
						, fprintf(
							stderr
							, "[s_cast_numeric::aclid_is_arithmetic] %u\n"
							, (unsigned)VEC[ACLEXPR_OPBINCFG_RESTYID]));

				h_ctx->m_err = ACLSPV_COMPILE_ARITHMETIC_CAST_TYPE_IS_UNDEFINED_NEW;
				return c_lbl_fail;
			}
	}

	dbg_prefix();
	dbg_call((void), fprintf(stderr, "[s_cast_numeric::aclutil_mk_cxtp_by_cursor] %u\n", VEC[ACLEXPR_OPBINCFG_RESTYID]));

	return c_lbl_success;
}

#endif
