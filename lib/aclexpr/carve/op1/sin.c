#include "./sin.h"

#include <aclexpr/carve/util.h>

#include <aclcur/opX.h>
#include <aclutil/extinst.h>

#include <aclty.h>

#if	__ACLSPV_MATH_LITERAL
#	include <math.h>
#	include <aclval.h>
#endif

#include <spirv/unified1/GLSL.std.450.h>

aclspv_bool_t aclexpr_carve1_sin(
		struct aclexpr_visit_data	ae2fcc_arg_sarr_restrict(rdwr_data, 1),
		aclspv_wrd_t			ae2fcc_arg_sarr_restrict(rdwr_prv_idval, 1),
		aclexpr_opbincfg_extra		ae2fcc_arg_sarr_restrict(rdwr_prv_extra, 1),
		aclspv_wrd_t			ae2fcc_arg_sarr_restrict(rdwr_lst_scale_buf, ACLEXPR_OPBINCFG_END + 1),
		aclexpr_opbincfg_extra		ae2fcc_arg_sarr_restrict(rdwr_lst_extra, 1),
		aclutil_literal			ae2fcc_arg_sarr_restrict(r_ltrl, 1)
		) {
	const h_aclspv_ctx_t	CTX = rdwr_data->m_ctx;
	const aclspv_id_t	TY = rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_RESTYID];
	aclspv_id_t		X = rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_END];
	aclspv_id_t		SET_ID;
	aclspv_id_t		ID;
	aclty_sclr_t		SCLR;
	aclspv_wrd_t		WIDTH;

	ae2f_ifezerr(TY) return ACLEXPR_CARVE_NO;
	ae2f_ifezerr(X)  return ACLEXPR_CARVE_NO;

	/** Sin requires a floating operand of the result type. */
	if(aclexpr_carve_cast_to_ty(TY, &X, &rdwr_lst_extra[0], CTX))
		return ACLEXPR_CARVE_NO;

	SCLR = aclty_get_simd_sclr(TY, CTX);
	ae2f_ifezerr(SCLR) return ACLEXPR_CARVE_NO;
	ae2f_ifezerr(aclty_is_sclr_flt(SCLR)) return ACLEXPR_CARVE_NO;

	/** GLSL.std.450 Sin supports 16/32-bit floats. only 32 is wired; f64
	 * is deferred. */
	WIDTH = (aclspv_wrd_t)aclty_get_sclr_width(SCLR);
	if(WIDTH != 32)
		return ACLEXPR_CARVE_NO;

#if	__ACLSPV_MATH_LITERAL
	/** scalar literal: fold on the host. OpExtInst is not a legal
	 * OpSpecConstantOp, so a constant context needs a plain OpConstant.
	 * vectors have no literal folding in any lane. */
	if(aclty_get_veccfg(TY, CTX).m_num == 1
			&& (rdwr_lst_extra[0].m_mask_literal
				& (aclspv_wrd_t)ACLUTIL_LTRL_LTRL)) {
		aclutil_literal	R = rdwr_lst_extra[0].m_literal;
		aclspv_id_t	FOLDED;

		R.m_flt = sinf(R.m_flt);

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
			, GLSLstd450Sin
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
