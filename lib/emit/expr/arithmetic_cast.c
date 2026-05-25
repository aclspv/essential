
#include <ae2f/c90/StdInt.h>
#include <ae2f/pat/dbg.h>
#include <ae2f/bll.h>

#include <util/literal.h>
#include <util/iddef.h>
#include <util/emitx.h>
#include <util/ctx.h>
#include <util/constant.h>
#include <util/cxtp.h>

#include "../expr.h"
#include "ae2f/cc.h"


static enum EMIT_EXPR_ s_non_literal_novec(
		const aclspv_id_t	c_old_id,
		const aclspv_id_t	c_old_type,
		const B_util_literal_t	c_flag_literal,
		aclspv_id_t* ae2f_restrict		r_newid,
		aclspv_id_t				c_new_type,
		const h_util_ctx_t			h_ctx,


		const aclspv_id_t	c_new_ty_eval_opt	/** actual type on emitting */
		)
{
	const aclspv_id_t 
		TYEVAL_NEW	= ae2f_bll_sel_bool_naive1(aclspv_id_t, c_new_ty_eval_opt, c_new_ty_eval_opt, c_new_type);

	ae2fpat_assertassume(r_newid);
	ae2fpat_assertassume(h_ctx);

	ae2f_ifezerr(c_old_type) {
		h_ctx->m_err = ACLSPV_COMPILE_ARITHMETIC_CAST_TYPE_IS_UNDEFINED_OLD;
		return EMIT_EXPR_FAILURE;
	}

	ae2f_ifezerr(c_new_type) {
		h_ctx->m_err = ACLSPV_COMPILE_ARITHMETIC_CAST_TYPE_IS_UNDEFINED_NEW;
		return EMIT_EXPR_FAILURE;
	}

	ae2f_ifezerr(c_old_id) {
		h_ctx->m_err = ACLSPV_COMPILE_ARITHMETIC_CAST_ID_IS_UNDEFINED_OLD;
		return EMIT_EXPR_FAILURE;
	}

	dbg_puts(("Arithmetic cast: non-literal"));

	/** two are same */
	if(c_old_type == c_new_type) {
		const uint_fastenum_t	CND = c_flag_literal & UTIL_LITERAL_CONSTANT;

		*r_newid = c_old_id;

		return (enum EMIT_EXPR_)ae2f_bll_sel_bool_naive1(
				uint_fastenum_t
				, CND
				, EMIT_EXPR_SUCCESS_CONSTANT
				, EMIT_EXPR_SUCCESS);
	}

	/** down here, two types are different */
	dbg_puts("two are not same");

	unless(aclid_is_vectorisable(c_old_type) && aclid_is_vectorisable(c_new_type))
		return EMIT_EXPR_NOT_THE_CASE;

	/** two are float */
	if (aclid_is_float(c_old_type) && aclid_is_float(c_new_type)) {
		if(c_flag_literal & UTIL_LITERAL_CONSTANT) {
			ae2f_expected_but_else(h_ctx->m_count.m_types
					= util_emitx_5(
						&h_ctx->m_section.m_types
						, h_ctx->m_count.m_types
						, SpvOpSpecConstantOp
						, TYEVAL_NEW
						, *r_newid = h_ctx->m_id++
						, SpvOpFConvert
						, c_old_id))
				return EMIT_EXPR_FAILURE;

			return EMIT_EXPR_SUCCESS_CONSTANT;
		} else {
			ae2f_expected_but_else(h_ctx->m_count.m_fnimpl
					= util_emitx_4(
						&h_ctx->m_section.m_fnimpl
						, h_ctx->m_count.m_fnimpl
						, SpvOpFConvert
						, TYEVAL_NEW
						, *r_newid = h_ctx->m_id++
						, c_old_id
						)
					) return EMIT_EXPR_FAILURE;
			return EMIT_EXPR_SUCCESS;
		}
	}


	dbg_puts("two are not float");

	if (aclid_is_int(c_old_type) && aclid_is_int(c_new_type)) {
		const aclspv_wrd_t
			IS_UNSIGNED = aclid_is_unsigned(c_new_type);

		const aclspv_wrd_t
			OPCODE = ae2f_bll_sel_bool1(
					aclspv_wrd_t
					, IS_UNSIGNED
					, SpvOpUConvert
					, SpvOpSConvert);

		unless(aclid_get_bit_width(c_old_type) != aclid_get_bit_width(c_new_type)) {
			const aclspv_wrd_t	VAL_ZER_CND = aclid_get_bit_width(c_old_type) == 64;
			const aclspv_id_t	VAL_ZERO = 
				ae2f_bll_sel_bool_naive1(
						aclspv_id_t
						, VAL_ZER_CND
						, aclutil_mk_constant_val64_id(0, h_ctx)
						, aclutil_mk_constant_val_id(0, h_ctx));

			ae2f_expected_but_else(VAL_ZERO) {
				return EMIT_EXPR_FAILURE;
			}

			if(c_flag_literal & UTIL_LITERAL_CONSTANT) {
				ae2f_expected_but_else(h_ctx->m_count.m_types
						= util_emitx_6(
							&h_ctx->m_section.m_types
							, h_ctx->m_count.m_types
							, SpvOpSpecConstantOp
							, TYEVAL_NEW
							, *r_newid = h_ctx->m_id++
							, SpvOpBitwiseOr 
							, c_old_id
							, VAL_ZERO))
					return EMIT_EXPR_FAILURE;

				return EMIT_EXPR_SUCCESS_CONSTANT;
			} else {
				ae2f_expected_but_else(h_ctx->m_count.m_fnimpl
						= util_emitx_5(
							&h_ctx->m_section.m_fnimpl
							, h_ctx->m_count.m_fnimpl
							, SpvOpBitwiseOr
							, TYEVAL_NEW
							, *r_newid = h_ctx->m_id++
							, c_old_id
							, VAL_ZERO
							)
						) return EMIT_EXPR_FAILURE;
				return EMIT_EXPR_SUCCESS;
			}

			ae2f_unreachable();
		}

		if(c_flag_literal & UTIL_LITERAL_CONSTANT) {
			ae2f_expected_but_else(h_ctx->m_count.m_types
					= util_emitx_5(
						&h_ctx->m_section.m_types
						, h_ctx->m_count.m_types
						, SpvOpSpecConstantOp
						, TYEVAL_NEW
						, *r_newid = h_ctx->m_id++
						, OPCODE 
						, c_old_id))
				return EMIT_EXPR_FAILURE;

			return EMIT_EXPR_SUCCESS_CONSTANT;
		} else {
			ae2f_expected_but_else(h_ctx->m_count.m_fnimpl
					= util_emitx_4(
						&h_ctx->m_section.m_fnimpl
						, h_ctx->m_count.m_fnimpl
						, OPCODE 
						, TYEVAL_NEW
						, *r_newid = h_ctx->m_id++
						, c_old_id
						)
					) return EMIT_EXPR_FAILURE;
			return EMIT_EXPR_SUCCESS;
		}

		ae2f_unreachable();
	}

	dbg_puts("two are different");

	if(c_new_type == ID_DEFAULT_BOOL && (aclid_is_int(c_old_type) | aclid_is_float(c_old_type))) {
		aclspv_id_t		V0;
		aclspv_wrd_t		OPCODE = ae2f_bll_sel_bool1(aclspv_wrd_t, aclid_is_int(c_old_type), SpvOpINotEqual, SpvOpFOrdNotEqual);
		
		ae2f_expected_but_else(V0 = aclutil_mk_constant_val_by_enum(0, h_ctx, (enum ID_DEFAULT_) c_old_type))
			return EMIT_EXPR_FAILURE;

		if(c_flag_literal & UTIL_LITERAL_CONSTANT) {
			ae2f_expected_but_else(h_ctx->m_count.m_types
					= util_emitx_6(
						&h_ctx->m_section.m_types
						, h_ctx->m_count.m_types
						, SpvOpSpecConstantOp
						, TYEVAL_NEW
						, *r_newid = h_ctx->m_id++
						,OPCODE 
						, c_old_id, V0))
				return EMIT_EXPR_FAILURE;

			return EMIT_EXPR_SUCCESS_CONSTANT;
		} else {
			ae2f_expected_but_else(h_ctx->m_count.m_fnimpl
					= util_emitx_5(
						&h_ctx->m_section.m_fnimpl
						, h_ctx->m_count.m_fnimpl
						,OPCODE 
						, TYEVAL_NEW
						, *r_newid = h_ctx->m_id++
						, c_old_id 
						, V0
						)
					) return EMIT_EXPR_FAILURE;
			return EMIT_EXPR_SUCCESS;
		}
	}

	if((aclid_is_int(c_new_type)) && c_old_type == ID_DEFAULT_BOOL) {
		aclspv_id_t		V0, V1;

		ae2f_expected_but_else(V0 = aclutil_mk_constant_val_by_enum(0, h_ctx, (enum ID_DEFAULT_) c_old_type))
			return EMIT_EXPR_FAILURE;

		ae2f_expected_but_else(V1 = aclutil_mk_constant_val_by_enum(1, h_ctx, (enum ID_DEFAULT_) c_old_type))
			return EMIT_EXPR_FAILURE;


		if(c_flag_literal & UTIL_LITERAL_CONSTANT) {
			ae2f_expected_but_else(h_ctx->m_count.m_types
					= util_emitx_7(
						&h_ctx->m_section.m_types
						, h_ctx->m_count.m_types
						, SpvOpSpecConstantOp
						, TYEVAL_NEW
						, *r_newid = h_ctx->m_id++
						, SpvOpSelect
						, c_old_id, V1, V0))
				return EMIT_EXPR_FAILURE;

			return EMIT_EXPR_SUCCESS_CONSTANT;
		} else {
			ae2f_expected_but_else(h_ctx->m_count.m_fnimpl
					= util_emitx_6(
						&h_ctx->m_section.m_fnimpl
						, h_ctx->m_count.m_fnimpl
						, SpvOpINotEqual
						, TYEVAL_NEW
						, *r_newid = h_ctx->m_id++
						, c_old_id
						, V1, V0
						)
					) return EMIT_EXPR_FAILURE;
			return EMIT_EXPR_SUCCESS;
		}
	}

	/** TWO ARE DIFFERENT */
	if(aclid_is_float(c_new_type)) {
		ae2f_expected_but_else(h_ctx->m_count.m_fnimpl
				= util_emitx_4(
					&h_ctx->m_section.m_fnimpl
					, h_ctx->m_count.m_fnimpl
					, SpvOpConvertSToF 
					, TYEVAL_NEW
					, *r_newid = h_ctx->m_id++
					, c_old_id
					)
				) return EMIT_EXPR_FAILURE;

		return EMIT_EXPR_SUCCESS;
	}

	if(aclid_is_int(c_new_type)) {
		*r_newid = h_ctx->m_id;
		ae2f_ifezerr(h_ctx->m_count.m_fnimpl
				= util_emitx_4(
					&h_ctx->m_section.m_fnimpl
					, h_ctx->m_count.m_fnimpl
					, SpvOpConvertFToS
					, c_new_type
					, h_ctx->m_id++
					, c_old_id
					)
				) return EMIT_EXPR_FAILURE;

		return EMIT_EXPR_SUCCESS;
	}

	return EMIT_EXPR_NOT_THE_CASE;
}

#include <util/vec.h>

static enum EMIT_EXPR_ s_non_literal_vec(
		const aclspv_id_t	c_old_id,
		const aclspv_id_t	c_old_type,
		const B_util_literal_t	c_flag_literal,
		aclspv_id_t* ae2f_restrict		r_newid,
		aclspv_id_t				c_new_type,
		const h_util_ctx_t			h_ctx
		) {
	struct aclutil_cxtp_veccfg_t 
		VCFG_OLD, VCFG_NEW;

	unless(aclid_is_arithmetic(c_old_type, h_ctx) && aclid_is_arithmetic(c_new_type, h_ctx)) {
		warn_puts("[s_non_literal_vec::no_arithmetic]");
		return EMIT_EXPR_NOT_THE_CASE;
	}

	/** old and new */
	VCFG_OLD = aclutil_cxtp_get_veccfg(c_old_type, h_ctx);
	VCFG_NEW = aclutil_cxtp_get_veccfg(c_new_type, h_ctx);

	unless(VCFG_OLD.m_num ^ VCFG_NEW.m_num) {
		dbg_puts("[s_non_literal_vec::s_non_literal_novec]");
		return s_non_literal_novec(
				c_old_id
				, VCFG_OLD.m_ty_elm
				, c_flag_literal & ~(aclspv_wrd_t)(UTIL_LITERAL_LITERAL)
				, r_newid
				, VCFG_NEW.m_ty_elm
				, h_ctx
				, c_new_type
				);
	} else if(VCFG_NEW.m_num > 1 && VCFG_OLD.m_num == 1) {
		aclspv_id_t	VECTYID = aclutil_mk_cxtp_vec(
				VCFG_OLD.m_ty_elm
				, VCFG_NEW.m_num
				, h_ctx
				);

		struct aclutil_mk_constant_repvec_t	REPVEC = aclutil_mk_constant_repvec_id(
				VECTYID, c_old_id, c_flag_literal, h_ctx
				);

		dbg_puts("\trepvec");

		ae2f_ifezerr(REPVEC.m_id) {
			h_ctx->m_err = ACLSPV_COMPILE_ALLOC_FAILED;
			return EMIT_EXPR_FAILURE;
		}

		return s_non_literal_novec(
				REPVEC.m_id
				, VCFG_OLD.m_ty_elm
				, REPVEC.m_literal_mask
				, r_newid
				, VCFG_NEW.m_ty_elm
				, h_ctx
				, c_new_type
				);
	} else return EMIT_EXPR_NOT_THE_CASE;

	ae2f_unreachable();
}

enum EMIT_EXPR_ aclexpr_arithmetic_cast_non_literal(
		const aclspv_id_t	c_old_id,
		const aclspv_id_t	c_old_type,
		const B_util_literal_t	c_flag_literal,
		aclspv_id_t* ae2f_restrict		r_newid,
		aclspv_id_t				c_new_type,
		const h_util_ctx_t			h_ctx
		) {
	enum EMIT_EXPR_ R = s_non_literal_novec(
			c_old_id
			, c_old_type
			, c_flag_literal
			, r_newid
			, c_new_type
			, h_ctx
			, 0
			);

	dbg_prefix(;);
	dbg_call((void), fprintf(stderr, "cast_non_literal %u %u\n", c_old_type, c_new_type));

	switch(R) {
		default:
			ae2f_unreachable();
		case EMIT_EXPR_NOT_THE_CASE:
			break;

		case EMIT_EXPR_SUCCESS:
		case EMIT_EXPR_SUCCESS_LITERAL:
		case EMIT_EXPR_SUCCESS_CONSTANT:
		case EMIT_EXPR_FAILURE:
			return R;
	}

	dbg_puts("[aclexpr_arithmetic_cast_non_literal::s_non_literal_vec]");

	return s_non_literal_vec(
			c_old_id
			, c_old_type
			, c_flag_literal
			, r_newid
			, c_new_type
			, h_ctx
			);
}

enum EMIT_EXPR_ aclexpr_arithmetic_cast_literal(
		const aclspv_id_t	c_old_type,
		const emit_expr_literal	c_old_literal,
		aclspv_id_t* ae2f_restrict	r_newid,
		aclspv_id_t			c_new_type,
		emit_expr_literal* ae2f_restrict	wr_new_literal_opt,
		const h_util_ctx_t		h_ctx
		) {
	emit_expr_literal	RESULT = c_old_literal;

	dbg_prefix(a);
	dbg_call(ae2f_NONE, (fprintf(
					stderr
					, "Arithmetic cast: literal old[%u] new [%u]\n"
					, c_old_type, c_new_type
				    )));

	ae2f_expected_but_else(aclid_is_vectorisable(c_old_type) && aclid_is_vectorisable(c_new_type)) {
		return EMIT_EXPR_NOT_THE_CASE;
	}

	ae2f_expected_but_else(aclid_mk_default_id(c_old_type, h_ctx) 
			&& aclid_mk_default_id(c_new_type, h_ctx))
		return EMIT_EXPR_NOT_THE_CASE;

	if(aclid_is_float(c_old_type) && aclid_is_int(c_new_type)) {
		dbg_puts(("Cast (float -> int)"));
		switch(aclid_get_bit_width(c_old_type)) {
			case 64:
				dbg_prefix();
				dbg_call(fprintf, (stderr, "Cast from literal(double): %f\n", (float)c_old_literal.m_dbl));
				RESULT.m_api_intmax = (imax)c_old_literal.m_dbl;
				break;
			case 32:
				dbg_prefix();
				dbg_call(fprintf, (stderr, "Cast from literal(float): %f\n", (float)c_old_literal.m_flt));
				RESULT.m_api_intmax = (imax)c_old_literal.m_flt;
				break;
			default:
				ae2fpat_assertassume(0 && "unknown float bit-width");
				return EMIT_EXPR_FAILURE;
		}

		dbg_prefix();
		dbg_call(fprintf, (stderr, "Result literal(intmax): %ld\n", (long)RESULT.m_api_intmax));
	}

	else if(aclid_is_int(c_old_type) && aclid_is_float(c_new_type)) {
		dbg_prefix(a);
		dbg_call((void), (fprintf(stderr, "Cast (int -> float): %ld\n", (long)c_old_literal.m_api_intmax)));
		switch(aclid_get_bit_width(c_new_type)) {
			case 64:
				RESULT.m_dbl = (double)c_old_literal.m_api_intmax;
				dbg_prefix();
				dbg_call(fprintf, (stderr, "Literal result(double): %f\n", (float)RESULT.m_dbl));
				break;

			case 32:
				RESULT.m_flt = (float)c_old_literal.m_api_intmax;
				dbg_prefix();
				dbg_call(fprintf, (stderr, "Literal result(float): %f\n", RESULT.m_flt));
				break;
			default:
				ae2fpat_assertassume(0 && "unknown float bit-width");
				return EMIT_EXPR_FAILURE;
		}
	}

	else if(aclid_is_float(c_old_type) && aclid_is_float(c_new_type) && c_old_type != c_new_type)
		switch(aclid_get_bit_width(c_new_type)) {
			case 64:
				RESULT.m_dbl = c_old_literal.m_flt;
				break;
			case 32:
				RESULT.m_flt = (float)c_old_literal.m_dbl;
				break;
			default:
				ae2fpat_assertassume(0 && "unknown float bit-width");
				return EMIT_EXPR_FAILURE;
		}
	else if(aclid_is_int(c_old_type) && c_new_type == ID_DEFAULT_BOOL) {
		RESULT.m_u64 = !!c_old_literal.m_api_uintmax;
		*r_newid = aclutil_mk_constant_bool_id((aclspv_bool_t)RESULT.m_u64, h_ctx);
		if(wr_new_literal_opt) *wr_new_literal_opt = RESULT;
		return EMIT_EXPR_SUCCESS_LITERAL;
	} else if(aclid_is_float(c_old_type) && c_new_type == ID_DEFAULT_BOOL) {
		switch(aclid_get_bit_width(c_old_type)) {
			case 64:
				RESULT.m_api_int = !!(i64_least)c_old_literal.m_dbl;
				break;
			case 32:
				RESULT.m_api_int = !!(i32_least)c_old_literal.m_flt;
				break;
			default:
				return EMIT_EXPR_NOT_THE_CASE;
		}

		*r_newid = aclutil_mk_constant_bool_id((aclspv_bool_t)RESULT.m_u64, h_ctx);
		RESULT.m_u64 = !!c_old_literal.m_api_uintmax;
		if(wr_new_literal_opt) *wr_new_literal_opt = RESULT;
		return EMIT_EXPR_SUCCESS_LITERAL;
	} else if (aclid_is_int(c_new_type) && c_old_type == ID_DEFAULT_BOOL) {
		RESULT.m_api_uintmax = c_old_literal.m_api_uintmax;
		*r_newid = aclutil_mk_constant_val_by_enum(
				RESULT.m_api_uintmax = c_old_literal.m_api_uintmax
				, h_ctx
				, (enum ID_DEFAULT_) c_new_type
				);
		if(wr_new_literal_opt) *wr_new_literal_opt = RESULT;
		return EMIT_EXPR_SUCCESS_LITERAL;
	} else if (aclid_is_float(c_new_type) && c_old_type == ID_DEFAULT_BOOL) {
		switch(aclid_get_bit_width(c_old_type)) {
			case 64:
				RESULT.m_dbl = (double)(c_old_literal.m_api_uintmax);
				break;
			case 32:
				RESULT.m_flt = (float)(c_old_literal.m_api_uintmax);
				break;
			default:
				return EMIT_EXPR_FAILURE;
		}

		if(wr_new_literal_opt) *wr_new_literal_opt = RESULT;
		*r_newid = aclutil_mk_constant_val_by_enum(
				RESULT.m_api_uintmax
				, h_ctx
				, (enum ID_DEFAULT_) c_new_type);
		return EMIT_EXPR_SUCCESS_LITERAL;
	}


	if(wr_new_literal_opt) *wr_new_literal_opt = RESULT;

	switch(c_new_type) {
		aclspv_id_t	ID_RET;

		case ID_DEFAULT_U32:
		case ID_DEFAULT_I32:
		ae2f_expected_but_else(ID_RET = aclutil_mk_constant_val_id(
					(aclspv_wrd_t)RESULT.m_api_uintmax
					, h_ctx)) return EMIT_EXPR_FAILURE;

		*r_newid = ID_RET;
		return EMIT_EXPR_SUCCESS_LITERAL;

		case ID_DEFAULT_U64:
		case ID_DEFAULT_I64:
		ae2f_expected_but_else(ID_RET = aclutil_mk_constant_val64_id(
					RESULT.m_api_uintmax
					, h_ctx)) return EMIT_EXPR_FAILURE;

		*r_newid = ID_RET;
		return EMIT_EXPR_SUCCESS_LITERAL;

		case ID_DEFAULT_F32:
		ae2f_expected_but_else(ID_RET = aclutil_mk_constant_val_f32_id(
					RESULT.m_flt
					, h_ctx)) return EMIT_EXPR_FAILURE;

		*r_newid = ID_RET;
		return EMIT_EXPR_SUCCESS_LITERAL;
		case ID_DEFAULT_F64:
		ae2f_expected_but_else(ID_RET = aclutil_mk_constant_val_f64_id(
					RESULT.m_dbl
					, h_ctx)) return EMIT_EXPR_FAILURE;
		*r_newid = ID_RET;
		return EMIT_EXPR_SUCCESS_LITERAL;

		default:
		fixme_puts("Type not found");
		return EMIT_EXPR_FAILURE;
	}

	ae2f_unreachable();

}

enum EMIT_EXPR_ aclexpr_arithmetic_cast(
		const aclspv_id_t	c_old_id,
		const aclspv_id_t	c_old_type,
		const B_util_literal_t	c_flag_literal,
		const emit_expr_literal	c_old_literal,

		aclspv_id_t* ae2f_restrict	r_newid,
		aclspv_id_t		c_new_type,
		emit_expr_literal* ae2f_restrict wr_new_literal_opt,
		const h_util_ctx_t	h_ctx
		)
{
	ae2fpat_assertassume(r_newid);
	ae2fpat_assertassume(h_ctx);

	ae2f_ifezerr(c_old_type) {
		h_ctx->m_err = ACLSPV_COMPILE_ARITHMETIC_CAST_TYPE_IS_UNDEFINED_OLD;
		return EMIT_EXPR_FAILURE;
	}

	ae2f_ifezerr(c_new_type) {
		h_ctx->m_err = ACLSPV_COMPILE_ARITHMETIC_CAST_TYPE_IS_UNDEFINED_NEW;
		return EMIT_EXPR_FAILURE;
	}

	ae2f_ifezerr(c_old_id) {
		h_ctx->m_err = ACLSPV_COMPILE_ARITHMETIC_CAST_ID_IS_UNDEFINED_OLD;
		return EMIT_EXPR_FAILURE;
	}

	(void)aclid_mk_default_id(c_new_type, h_ctx);
	(void)aclid_mk_default_id(c_old_type, h_ctx);

	if(c_flag_literal & UTIL_LITERAL_LITERAL) {
		const enum EMIT_EXPR_ R = aclexpr_arithmetic_cast_literal(
				c_old_type
				, c_old_literal
				, r_newid
				, c_new_type
				, wr_new_literal_opt
				, h_ctx);

		if(R == EMIT_EXPR_NOT_THE_CASE) aclexpr_arithmetic_cast_non_literal(
			c_old_id
			, c_old_type
			, c_flag_literal, r_newid
			, c_new_type, h_ctx
			);
	}

	return aclexpr_arithmetic_cast_non_literal(
			c_old_id
			, c_old_type
			, c_flag_literal, r_newid
			, c_new_type, h_ctx
			);
}
