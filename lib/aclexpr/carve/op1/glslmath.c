#include "./glslmath.h"

#include <aclexpr/carve/util.h>

#include <aclcur/opX.h>
#include <aclutil/extinst.h>

#include <aclty.h>

#include <spirv/unified1/GLSL.std.450.h>

#if	__ACLSPV_MATH_LITERAL
#	include <math.h>
#	include <aclval.h>
#endif

#if	__ACLSPV_MATH_LITERAL
/** host fold for an f32 literal operand. */
static aclspv_bool_t s_fold(aclspv_wrd_t c_inst, float* ae2f_restrict rw) {
	switch(c_inst) {
		case GLSLstd450Sin:	*rw = sinf(*rw); return 1;
		case GLSLstd450Cos:	*rw = cosf(*rw); return 1;
		case GLSLstd450Tan:	*rw = tanf(*rw); return 1;
		default:		return 0;
	}
}
#endif

/** shared body: operand conversion, width policy, scalar f32 literal fold,
 * then OpExtInst for the rest. */
static aclspv_bool_t s_carve1_glsl_math(
		aclspv_wrd_t			c_inst,
		struct aclexpr_visit_data* const ae2f_restrict	rdwr_data,
		aclspv_wrd_t* const ae2f_restrict		rdwr_prv_idval,
		aclexpr_opbincfg_extra* const ae2f_restrict	rdwr_prv_extra,
		aclspv_wrd_t* const ae2f_restrict		rdwr_lst_scale_buf,
		aclexpr_opbincfg_extra* const ae2f_restrict	rdwr_lst_extra,
		aclutil_literal* const ae2f_restrict		r_ltrl
		) {
	const h_aclspv_ctx_t	CTX = rdwr_data->m_ctx;
	/** the visit passes the builtin's expected operand type, or 0 for a
	 * generic spelling. */
	const aclspv_id_t	EXPECTED = rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_RESTYID];
	aclspv_id_t		TY = rdwr_lst_extra[0].m_type_original;
	aclspv_id_t		X = rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_END];
	aclspv_id_t		SET_ID;
	aclspv_id_t		ID;
	aclty_sclr_t		SCLR;
	aclspv_wrd_t		WIDTH;

	ae2f_ifezerr(TY) return ACLEXPR_CARVE_NO;
	ae2f_ifezerr(X)  return ACLEXPR_CARVE_NO;

	if(EXPECTED) {
		/** the builtin's signature fixes the operand type. */
		if(aclexpr_carve_cast_to_ty(EXPECTED, &X, &rdwr_lst_extra[0], CTX)) {
			CTX->m_err = ACLSPV_COMPILE_UNHANDLED_CARVE;
			return ACLEXPR_CARVE_NO;
		}

		TY = EXPECTED;
	} else {
		/** generic builtin: expect a simd float. a non-float operand is
		 * retargeted to the configured default scalar, same shape:
		 * i16x4 -> f32x4 with the default f32. */
		SCLR = aclty_get_simd_sclr(TY, CTX);

		if(!SCLR || !aclty_is_sclr_flt(SCLR)) {
			const aclspv_id_t DEF = aclty_simd_chxg_sclr(
					TY
					, (aclty_sclr_t)__ACLSPV_MATH_DEFUALT
					, CTX);

			if(!DEF
					|| aclexpr_carve_cast_to_ty(
						DEF, &X, &rdwr_lst_extra[0], CTX)) {
				CTX->m_err = ACLSPV_COMPILE_UNHANDLED_CARVE;
				return ACLEXPR_CARVE_NO;
			}

			TY = DEF;
		}
	}

	SCLR = aclty_get_simd_sclr(TY, CTX);

	if(!SCLR || !aclty_is_sclr_flt(SCLR)) {
		/** not a simd float: user-facing reject, not a silent pass. */
		CTX->m_err = ACLSPV_COMPILE_UNHANDLED_CARVE;
		return ACLEXPR_CARVE_NO;
	}

	/** GLSL.std.450 math supports 16/32-bit floats; f64 is deferred. */
	WIDTH = (aclspv_wrd_t)aclty_get_sclr_width(SCLR);

	if(WIDTH != 16 && WIDTH != 32) {
		CTX->m_err = ACLSPV_COMPILE_UNHANDLED_CARVE;
		return ACLEXPR_CARVE_NO;
	}

#if	__ACLSPV_MATH_LITERAL
	/** scalar f32 literal: fold on the host. OpExtInst is not a legal
	 * OpSpecConstantOp, so a constant context needs a plain OpConstant.
	 * f16 literal folding is deferred; vectors have no literal folding. */
	if(WIDTH == 32
			&& aclty_get_veccfg(TY, CTX).m_num == 1
			&& (rdwr_lst_extra[0].m_mask_literal
				& (aclspv_wrd_t)ACLUTIL_LTRL_LTRL)) {
		aclutil_literal	R = rdwr_lst_extra[0].m_literal;
		aclspv_id_t	FOLDED;

		if(!s_fold(c_inst, &R.m_flt))
			return ACLEXPR_CARVE_NO;

		ae2f_ifezerr(FOLDED = aclval_cfg_f32(R.m_flt, CTX))
			return ACLEXPR_CARVE_NO;

		*r_ltrl = R;
		rdwr_prv_extra->m_literal = R;
		rdwr_prv_extra->m_type_original = TY;
		rdwr_prv_extra->m_mask_literal = ACLUTIL_LTRL_MASK_LTRL;
		*rdwr_prv_idval = FOLDED;

		return ACLEXPR_CARVE_OK;
	}
#endif

	SET_ID = aclutil_glsl_extinst(CTX);
	ae2f_ifezerr(SET_ID) return ACLEXPR_CARVE_NO;

	ID = aclcur_cfg_extinst(
			0
			, TY
			, SET_ID
			, c_inst
			, &X
			, 1
			, CTX);
	ae2f_ifezerr(ID) return ACLEXPR_CARVE_NO;

	*rdwr_prv_idval = ID;
	rdwr_prv_extra->m_type_original = TY;
	rdwr_prv_extra->m_mask_literal = 0;
	rdwr_prv_extra->m_literal.m_api_uintmax = 0;
	*r_ltrl = rdwr_prv_extra->m_literal;

	return ACLEXPR_CARVE_OK;
}

	aclspv_bool_t aclexpr_carve1_sin(
			struct aclexpr_visit_data	ae2fcc_arg_sarr_restrict(rdwr_data, 1),
			aclspv_wrd_t			ae2fcc_arg_sarr_restrict(rdwr_prv_idval, 1),
			aclexpr_opbincfg_extra		ae2fcc_arg_sarr_restrict(rdwr_prv_extra, 1),
			aclspv_wrd_t			ae2fcc_arg_sarr_restrict(rdwr_lst_scale_buf, ACLEXPR_OPBINCFG_END + 1),
			aclexpr_opbincfg_extra		ae2fcc_arg_sarr_restrict(rdwr_lst_extra, 1),
			aclutil_literal			ae2fcc_arg_sarr_restrict(r_ltrl, 1)
			) {
		return s_carve1_glsl_math(
				GLSLstd450Sin
				, rdwr_data, rdwr_prv_idval, rdwr_prv_extra
				, rdwr_lst_scale_buf, rdwr_lst_extra, r_ltrl);
	}

	aclspv_bool_t aclexpr_carve1_cos(
			struct aclexpr_visit_data	ae2fcc_arg_sarr_restrict(rdwr_data, 1),
			aclspv_wrd_t			ae2fcc_arg_sarr_restrict(rdwr_prv_idval, 1),
			aclexpr_opbincfg_extra		ae2fcc_arg_sarr_restrict(rdwr_prv_extra, 1),
			aclspv_wrd_t			ae2fcc_arg_sarr_restrict(rdwr_lst_scale_buf, ACLEXPR_OPBINCFG_END + 1),
			aclexpr_opbincfg_extra		ae2fcc_arg_sarr_restrict(rdwr_lst_extra, 1),
			aclutil_literal			ae2fcc_arg_sarr_restrict(r_ltrl, 1)
			) {
		return s_carve1_glsl_math(
				GLSLstd450Cos
				, rdwr_data, rdwr_prv_idval, rdwr_prv_extra
				, rdwr_lst_scale_buf, rdwr_lst_extra, r_ltrl);
	}

	aclspv_bool_t aclexpr_carve1_tan(
			struct aclexpr_visit_data	ae2fcc_arg_sarr_restrict(rdwr_data, 1),
			aclspv_wrd_t			ae2fcc_arg_sarr_restrict(rdwr_prv_idval, 1),
			aclexpr_opbincfg_extra		ae2fcc_arg_sarr_restrict(rdwr_prv_extra, 1),
			aclspv_wrd_t			ae2fcc_arg_sarr_restrict(rdwr_lst_scale_buf, ACLEXPR_OPBINCFG_END + 1),
			aclexpr_opbincfg_extra		ae2fcc_arg_sarr_restrict(rdwr_lst_extra, 1),
			aclutil_literal			ae2fcc_arg_sarr_restrict(r_ltrl, 1)
			) {
		return s_carve1_glsl_math(
				GLSLstd450Tan
				, rdwr_data, rdwr_prv_idval, rdwr_prv_extra
				, rdwr_lst_scale_buf, rdwr_lst_extra, r_ltrl);
	}
