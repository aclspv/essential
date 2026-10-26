#include <ae2f/pat/dbg.h>
#include <ae2f/bll.h>
#include <ae2f/cc/arg_sarr.h>

#include <util/literal.h>
#include <util/iddef.h>
#include <util/emitx.h>
#include <util/wrdemit.h>

#include <aclty.h>
#include <aclty/simd/cls.h>
#include <aclval.h>
#include <aclval/nil.h>
#include <aclexpr.h>
#include <aclconst/specop.h>
#include <aclcur/common.h>

#include <aclctx/id.h>
#include <aclctx/err.h>

/** c_old_type and c_new_type's shape MUST BE SAME */
static enum ACLEXPR_RET_ s_cast_arithmetic(
		const aclspv_id_t	c_old_id,
		const aclspv_id_t	c_old_type,
		const aclutil_ltrlmask_t	c_flag_literal,
		aclspv_id_t			ae2fcc_arg_sarr_restrict(r_newid, 1),
		aclspv_id_t				c_new_type,
		const h_aclspv_ctx_t			h_ctx
		)
{
	ae2fpat_assertassume(h_ctx);

	/** two are float */
	if (aclty_is_simd_flt(c_old_type, h_ctx) && aclty_is_simd_flt(c_new_type, h_ctx)) {
		if(c_flag_literal & ACLUTIL_LITERAL_CONSTANT) {
			aclspv_wrd_t buf[5];
			buf[0] = aclspv_opcode_to_wrd(SpvOpSpecConstantOp, 4);
			buf[1] = c_new_type;
			buf[3] = SpvOpFConvert;
			buf[4] = c_old_id;

			ae2f_ifezerr(*r_newid = aclconst_specop(
						&buf[0]
						, 5, h_ctx)) {
				return ACLEXPR_RET_FAILURE;
			}

			return ACLEXPR_RET_SUCCESS_CONSTANT;
		} else {
			ae2f_ifezerr(*r_newid = aclcur_cfg_op1(
						aclctx_newid(h_ctx)
					, SpvOpFConvert
					, c_new_type
					, c_old_id
					, h_ctx
					))
				return ACLEXPR_RET_FAILURE;
			return ACLEXPR_RET_SUCCESS;
		}
	}


	if (aclty_is_simd_int(c_old_type, h_ctx) && aclty_is_simd_int(c_new_type, h_ctx)) {
		const aclspv_wrd_t
			OPCODE = aclty_is_simd_uint(c_new_type, h_ctx) ? SpvOpUConvert : SpvOpSConvert;

		unless(aclty_get_simd_bit_width(c_old_type, h_ctx) != aclty_get_simd_bit_width(c_new_type, h_ctx)) {
			const aclspv_id_t	VAL_ZERO = aclval_cfg_nil(c_old_type, h_ctx); 

			ae2f_ifezerr(VAL_ZERO) {
				err_puts("val_zero\n");
				return ACLEXPR_RET_FAILURE;
			}

			if(c_flag_literal & ACLUTIL_LITERAL_CONSTANT) {
				aclspv_wrd_t buf[6];
				buf[0] = aclspv_opcode_to_wrd(SpvOpSpecConstantOp, 5);
				buf[1] = c_new_type;
				buf[3] = SpvOpBitwiseOr;
				buf[4] = c_old_id;
				buf[5] = VAL_ZERO;

				ae2f_ifezerr(*r_newid = aclconst_specop(buf, 6, h_ctx)) {
					return ACLEXPR_RET_FAILURE;
				}

				return ACLEXPR_RET_SUCCESS_CONSTANT;
			} else {
				ae2f_ifezerr(*r_newid = aclcur_cfg_op2(
							aclctx_newid(h_ctx)
							, SpvOpBitwiseOr
							, c_new_type
							, c_old_id
							, VAL_ZERO
							, h_ctx
							))
					return ACLEXPR_RET_FAILURE;
				return ACLEXPR_RET_SUCCESS;
			}

			ae2f_unreachable();
		}

		if(c_flag_literal & ACLUTIL_LITERAL_CONSTANT) {
			aclspv_wrd_t buf[5];
			buf[0] = aclspv_opcode_to_wrd(SpvOpSpecConstantOp, 4);
			buf[1] = c_new_type;
			buf[3] = OPCODE;
			buf[4] = c_old_id;

			ae2f_ifezerr(*r_newid = aclconst_specop(buf, 5, h_ctx))
				return ACLEXPR_RET_FAILURE;

			return ACLEXPR_RET_SUCCESS_CONSTANT;
		} else {
			ae2f_ifezerr(*r_newid = aclcur_cfg_op1(
						aclctx_newid(h_ctx)
						, (enum SpvOp_)OPCODE
						, c_new_type
						, c_old_id
						, h_ctx
						))
				return ACLEXPR_RET_FAILURE;
			return ACLEXPR_RET_SUCCESS;
		}

		ae2f_unreachable();
	}

	if(aclty_simd_class(c_new_type, h_ctx) == ACLSPV_ID_BOOL 
			&& (aclty_is_simd_int(c_old_type, h_ctx) || aclty_is_simd_flt(c_old_type, h_ctx))) {
		aclspv_id_t	V0 = aclval_cfg_nil(c_old_type, h_ctx);
		aclspv_wrd_t	OPCODE = aclty_is_simd_int(c_old_type, h_ctx)
			? SpvOpINotEqual : SpvOpFOrdNotEqual;

		ae2f_ifezerr(V0)
			return ACLEXPR_RET_FAILURE;

		if(c_flag_literal & ACLUTIL_LITERAL_CONSTANT) {
			aclspv_wrd_t buf[6];
			buf[0] = aclspv_opcode_to_wrd(SpvOpSpecConstantOp, 5);
			buf[1] = c_new_type;

			buf[3] = OPCODE;
			buf[4] = c_old_id;
			buf[5] = V0;

			ae2f_ifezerr(aclconst_specop(buf, 6, h_ctx))
				return ACLEXPR_RET_FAILURE;

			return ACLEXPR_RET_SUCCESS_CONSTANT;
		} else {
			ae2f_ifezerr(*r_newid = aclcur_cfg_op2(
						aclctx_newid(h_ctx)
						, (enum SpvOp_)OPCODE
						, c_new_type
						, c_old_id
						, V0
						, h_ctx
						))
				return ACLEXPR_RET_FAILURE;
			return ACLEXPR_RET_SUCCESS;
		}
	}

	if((aclty_is_simd_int(c_new_type, h_ctx)) &&  aclty_simd_class(c_old_type, h_ctx) == ACLSPV_ID_BOOL) {
		aclspv_id_t		V0, V1, V1_CLS;

		ae2f_ifezerr(V0 = aclval_cfg_nil(c_new_type, h_ctx))
			return ACLEXPR_RET_FAILURE;

		ae2f_ifezerr(V1_CLS = aclval_cfg_by_enum(1, h_ctx, (enum ACLSPV_ID_) aclty_simd_class(c_new_type, h_ctx)))
			return ACLEXPR_RET_FAILURE;

		ae2f_ifezerr(V1 = aclty_simd_broadcast(V1_CLS, c_new_type, ACLUTIL_LITERAL_CONSTANT, h_ctx))
			return ACLEXPR_RET_FAILURE;

		if(c_flag_literal & ACLUTIL_LITERAL_CONSTANT) {
			aclspv_wrd_t buf[7];
			buf[0] = aclspv_opcode_to_wrd(SpvOpSpecConstantOp, 6);
			buf[1] = c_new_type;

			buf[3] = SpvOpSelect;
			buf[4] = c_old_id;
			buf[5] = V1;
			buf[6] = V0;


			ae2f_ifezerr(*r_newid = aclconst_specop(buf, 7, h_ctx))
				return ACLEXPR_RET_FAILURE;

			return ACLEXPR_RET_SUCCESS_CONSTANT;
		} else {
			ae2f_expected_but_else(*r_newid = aclcur_cfg_op3(
						aclctx_newid(h_ctx)
						, SpvOpSelect
						, c_new_type
						, c_old_id
						, V1, V0
						, h_ctx
						))
				return ACLEXPR_RET_FAILURE;
			return ACLEXPR_RET_SUCCESS;
		}
	}

	/** TWO ARE DIFFERENT */
	if(aclty_is_simd_flt(c_new_type, h_ctx)) {
		ae2f_expected_but_else(*r_newid = aclcur_cfg_op1(
					aclctx_newid(h_ctx)
					, SpvOpConvertSToF
					, c_new_type
					, c_old_id
					, h_ctx
					))
			return ACLEXPR_RET_FAILURE;

		return ACLEXPR_RET_SUCCESS;
	}

	if(aclty_is_simd_int(c_new_type, h_ctx)) {
		ae2f_expected_but_else(*r_newid = aclcur_cfg_op1(
					aclctx_newid(h_ctx)
					, SpvOpConvertFToS
					, c_new_type
					, c_old_id
					, h_ctx
					))
			return ACLEXPR_RET_FAILURE;

		return ACLEXPR_RET_SUCCESS;
	}

	return ACLEXPR_RET_NOT_THE_CASE;
}

enum ACLEXPR_RET_ aclexpr_arithmetic_cast_non_literal(
		const aclspv_id_t	c_old_id,
		const aclspv_id_t	c_old_type,
		const aclutil_ltrlmask_t	c_flag_literal,
		aclspv_id_t* ae2f_restrict		r_newid,
		aclspv_id_t				c_new_type,
		const h_aclspv_ctx_t			h_ctx
		) {
	if(aclty_simd_chxg_cls(
				c_old_type
				, aclty_simd_class(c_new_type, h_ctx)
				, h_ctx) == c_new_type) {
		return s_cast_arithmetic(c_old_id, c_old_type, c_flag_literal, r_newid, c_new_type, h_ctx);
	}

	ae2f_ifneznrm(aclty_is_simd_class(c_old_type)) {
		const aclspv_id_t old_ty_shaped = aclty_simd_chxg_cls(c_new_type, c_old_type, h_ctx);
		return s_cast_arithmetic(
				aclty_simd_broadcast(
					c_old_id
					, old_ty_shaped
					, c_flag_literal
					, h_ctx)
				, old_ty_shaped
				, c_flag_literal & ~(aclspv_wrd_t)ACLUTIL_LITERAL_LITERAL
				, r_newid
				, c_new_type
				, h_ctx
				);

	} else return ACLEXPR_RET_FAILURE;
}

enum ACLEXPR_RET_ aclexpr_arithmetic_cast_literal(
		const aclspv_id_t	c_old_type,
		const aclexpr_literal	c_old_literal,
		aclspv_id_t* ae2f_restrict	r_newid,
		aclspv_id_t			c_new_type,
		aclexpr_literal* ae2f_restrict	wr_new_literal_opt,
		const h_aclspv_ctx_t		h_ctx
		) {
	aclexpr_literal	RESULT = c_old_literal;

	dbg_prefix(a);
	dbg_call(ae2f_NONE, (fprintf(
					stderr
					, "Arithmetic cast: literal old[%u] new [%u]\n"
					, c_old_type, c_new_type
				    )));

	ae2f_expected_but_else(aclid_mk_default_id(c_old_type, h_ctx) 
			&& aclid_mk_default_id(c_new_type, h_ctx))
		return ACLEXPR_RET_NOT_THE_CASE;

	dbg_puts("start literal cast evaluation");
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
				return ACLEXPR_RET_FAILURE;
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
				return ACLEXPR_RET_FAILURE;
		}
	}

	else if(aclid_is_float(c_old_type) && aclid_is_float(c_new_type) && c_old_type != c_new_type) {
		dbg_puts("two are float");
		switch(aclid_get_bit_width(c_new_type)) {
			case 64:
				RESULT.m_dbl = c_old_literal.m_flt;
				break;
			case 32:
				RESULT.m_flt = (float)c_old_literal.m_dbl;
				break;
			default:
				ae2fpat_assertassume(0 && "unknown float bit-width");
				return ACLEXPR_RET_FAILURE;
		}
	}
	else if(aclid_is_int(c_old_type) && c_new_type == ACLSPV_ID_BOOL) {
		dbg_puts("int -> bool");
		RESULT.m_u64 = !!c_old_literal.m_api_uintmax;
		*r_newid = aclval_cfg_bool((aclspv_bool_t)RESULT.m_u64, h_ctx);
		if(wr_new_literal_opt) *wr_new_literal_opt = RESULT;
		return ACLEXPR_RET_SUCCESS_LITERAL;
	} else if(aclid_is_float(c_old_type) && c_new_type == ACLSPV_ID_BOOL) {
		dbg_puts("float -> bool");
		switch(aclid_get_bit_width(c_old_type)) {
			case 64:
				RESULT.m_api_int = !!(i64_least)c_old_literal.m_dbl;
				break;
			case 32:
				RESULT.m_api_int = !!(i32_least)c_old_literal.m_flt;
				break;
			default:
				return ACLEXPR_RET_NOT_THE_CASE;
		}

		*r_newid = aclval_cfg_bool((aclspv_bool_t)RESULT.m_u64, h_ctx);
		RESULT.m_u64 = !!c_old_literal.m_api_uintmax;
		if(wr_new_literal_opt) *wr_new_literal_opt = RESULT;
		return ACLEXPR_RET_SUCCESS_LITERAL;
	} else if (aclid_is_int(c_new_type) && c_old_type == ACLSPV_ID_BOOL) {
		dbg_puts("bool -> int");
		RESULT.m_api_uintmax = c_old_literal.m_api_uintmax;
		*r_newid = aclval_cfg_by_enum(
				RESULT.m_api_uintmax = c_old_literal.m_api_uintmax
				, h_ctx
				, (enum ACLSPV_ID_) c_new_type
				);
		if(wr_new_literal_opt) *wr_new_literal_opt = RESULT;
		return ACLEXPR_RET_SUCCESS_LITERAL;
	} else if (aclid_is_float(c_new_type) && c_old_type == ACLSPV_ID_BOOL) {
		dbg_puts("bool -> float");
		switch(aclid_get_bit_width(c_old_type)) {
			case 64:
				RESULT.m_dbl = (double)(c_old_literal.m_api_uintmax);
				break;
			case 32:
				RESULT.m_flt = (float)(c_old_literal.m_api_uintmax);
				break;
			default:
				return ACLEXPR_RET_FAILURE;
		}

		if(wr_new_literal_opt) *wr_new_literal_opt = RESULT;
		*r_newid = aclval_cfg_by_enum(
				RESULT.m_api_uintmax
				, h_ctx
				, (enum ACLSPV_ID_) c_new_type);
		return ACLEXPR_RET_SUCCESS_LITERAL;
	}

	if(wr_new_literal_opt) *wr_new_literal_opt = RESULT;

	switch(c_new_type) {
		aclspv_id_t	ID_RET;

		case ACLSPV_ID_U32:
		case ACLSPV_ID_I32:
		ae2f_expected_but_else(ID_RET = aclutil_mk_constant_val_id(
					(aclspv_wrd_t)RESULT.m_api_uintmax
					, h_ctx)) return ACLEXPR_RET_FAILURE;

		*r_newid = ID_RET;
		return ACLEXPR_RET_SUCCESS_LITERAL;

		case ACLSPV_ID_U64:
		case ACLSPV_ID_I64:
		ae2f_expected_but_else(ID_RET = aclutil_mk_constant_val64_id(
					RESULT.m_api_uintmax
					, h_ctx)) return ACLEXPR_RET_FAILURE;

		*r_newid = ID_RET;
		return ACLEXPR_RET_SUCCESS_LITERAL;

		case ACLSPV_ID_F32:
		ae2f_expected_but_else(ID_RET = aclval_cfg_f32(
					RESULT.m_flt
					, h_ctx)) return ACLEXPR_RET_FAILURE;

		*r_newid = ID_RET;
		return ACLEXPR_RET_SUCCESS_LITERAL;
		case ACLSPV_ID_F64:
		ae2f_expected_but_else(ID_RET = aclval_cfg_f64(
					RESULT.m_dbl
					, h_ctx)) return ACLEXPR_RET_FAILURE;
		*r_newid = ID_RET;
		return ACLEXPR_RET_SUCCESS_LITERAL;

		case ACLSPV_ID_BOOL:
		ae2f_ifezerr(ID_RET = aclval_cfg_bool((aclspv_wrd_t)!!RESULT.m_api_uintmax, h_ctx))
			return ACLEXPR_RET_FAILURE;

		*r_newid = ID_RET;
		return ACLEXPR_RET_SUCCESS_LITERAL;

		default:
		err_prefix();
		err_call((void), fprintf(stderr, "Type not found %u\n", (unsigned)c_new_type));
		return ACLEXPR_RET_FAILURE;
	}

	ae2f_unreachable();

}

enum ACLEXPR_RET_ aclexpr_arithmetic_cast(
		const aclspv_id_t	c_old_id,
		const aclspv_id_t	c_old_type,
		const aclutil_ltrlmask_t	c_flag_literal,
		const aclexpr_literal	c_old_literal,

		aclspv_id_t* ae2f_restrict	r_newid,
		aclspv_id_t		c_new_type,
		aclexpr_literal* ae2f_restrict r_new_literal_opt,
		const h_aclspv_ctx_t	h_ctx
		)
{
	enum ACLEXPR_RET_	R;

	ae2fpat_assertassume(r_newid);
	ae2fpat_assertassume(h_ctx);

	/** two are same */
	if(c_old_type == c_new_type) {
		const uint_fastenum_t	CND_LIT = c_flag_literal & ACLUTIL_LITERAL_LITERAL;
		*r_newid = c_old_id;

		if(r_new_literal_opt)
			*r_new_literal_opt = c_old_literal;

		if (CND_LIT)
			return ACLEXPR_RET_SUCCESS_LITERAL;  /** prefer LITERAL so mask stays 0x3; prevents float literal arith from being forced to RT path after clear in carve2_arithmetic_do */
		return (c_flag_literal & ACLUTIL_LITERAL_CONSTANT) ? ACLEXPR_RET_SUCCESS_CONSTANT : ACLEXPR_RET_SUCCESS;
	}

	ae2f_ifezerr(c_old_type) {
		_aclctx_err(h_ctx, ACLSPV_COMPILE_ARITHMETIC_CAST_TYPE_IS_UNDEFINED_OLD);
		return ACLEXPR_RET_FAILURE;
	}

	ae2f_ifezerr(c_new_type) {
		_aclctx_err(h_ctx, ACLSPV_COMPILE_ARITHMETIC_CAST_TYPE_IS_UNDEFINED_NEW);
		return ACLEXPR_RET_FAILURE;
	}

	ae2f_ifezerr(c_old_id) {
		_aclctx_err(h_ctx, ACLSPV_COMPILE_ARITHMETIC_CAST_TYPE_IS_UNDEFINED_OLD);
		return ACLEXPR_RET_FAILURE;
	}

	ae2f_ifnezerr(!aclty_is_simd(c_old_type, h_ctx) || !aclty_is_simd(c_new_type, h_ctx)) {
		warn_puts("no arithmetic");
		return ACLEXPR_RET_NOT_THE_CASE;
	}

	(void)aclid_mk_default_id(c_new_type, h_ctx);
	(void)aclid_mk_default_id(c_old_type, h_ctx);


	dbg_prefix();
	dbg_call((void), fprintf(stderr, "[arithmetic_cast] (%u : %u) -> (? : %u)\n"
				, (unsigned) c_old_id
				, (unsigned) c_old_type
				, (unsigned) c_new_type
				));

	return  (R = ((c_flag_literal & ACLUTIL_LITERAL_LITERAL) ? aclexpr_arithmetic_cast_literal(
					c_old_type
					, c_old_literal
					, r_newid
					, c_new_type
					, r_new_literal_opt
					, h_ctx
					) : ACLEXPR_RET_NOT_THE_CASE)) == ACLEXPR_RET_NOT_THE_CASE
		? aclexpr_arithmetic_cast_non_literal(
				c_old_id
				, c_old_type
				, c_flag_literal, r_newid
				, c_new_type, h_ctx
				) : R;
}
