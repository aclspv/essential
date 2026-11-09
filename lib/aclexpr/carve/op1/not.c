#include "./not.h"

#include <aclexpr/carve/util.h>

#include <aclcur/op.h>

#include <aclty.h>
#include <aclval/nil.h>

/** `!x`.
 *
 * semantically `x == 0`: an ordered equality, so `!NaN` is false like C
 * (a `!= 0` cast followed by a negation would say true for NaN).
 *
 * scalar: the comparison yields bool; the C int result comes from the
 * implicit cast at the use site.
 *
 * vector: C/OpenCL define `!` elementwise, yielding -1 where the element is
 * zero and 0 elsewhere, so select an all-ones vector against zero.
 * */
aclspv_bool_t aclexpr_carve1_not(
		struct aclexpr_visit_data	ae2fcc_arg_sarr_restrict(rdwr_data, 1),
		aclspv_wrd_t			ae2fcc_arg_sarr_restrict(rdwr_prv_idval, 1),
		aclexpr_opbincfg_extra		ae2fcc_arg_sarr_restrict(rdwr_prv_extra, 1),
		aclspv_wrd_t			ae2fcc_arg_sarr_restrict(rdwr_lst_scale_buf, ACLEXPR_OPBINCFG_END + 1),
		aclexpr_opbincfg_extra		ae2fcc_arg_sarr_restrict(rdwr_lst_extra, 1),
		aclutil_literal			ae2fcc_arg_sarr_restrict(r_ltrl, 1)
		) {
	const h_aclspv_ctx_t	CTX = rdwr_data->m_ctx;
	const aclspv_id_t	TY = rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_RESTYID];
	const aclspv_id_t	OPND_TY = rdwr_lst_extra[0].m_type_original;
	aclspv_id_t		X = rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_END];
	aclspv_id_t		BOOL_TY;
	aclspv_id_t		SCLR;
	aclspv_id_t		ZERO;
	aclspv_id_t		ID;
	enum SpvOp_		EQOP;
	struct aclutil_cxtp_veccfg_t V;

	ae2f_ifezerr(TY)	return ACLEXPR_CARVE_NO;
	ae2f_ifezerr(X)		return ACLEXPR_CARVE_NO;
	ae2f_ifezerr(OPND_TY)	return ACLEXPR_CARVE_NO;

	SCLR = aclty_get_simd_sclr(OPND_TY, CTX);
	ae2f_ifezerr(SCLR) return ACLEXPR_CARVE_NO;

	V = aclty_get_veccfg(TY, CTX);

	BOOL_TY = aclty_simd_chxg_sclr(TY, ACLSPV_ID_BOOL, CTX);
	ae2f_ifezerr(BOOL_TY) return ACLEXPR_CARVE_NO;

	EQOP = aclty_is_sclr_flt(SCLR) ? SpvOpFOrdEqual : SpvOpIEqual;

	/** zero of the operand's own type: `x == 0` is exactly `!x`. */
	ae2f_ifezerr(ZERO = aclval_cfg_nil(OPND_TY, CTX))
		return ACLEXPR_CARVE_NO;

	ae2f_ifezerr(ID = aclcur_cfg_op2(
				0
				, EQOP
				, BOOL_TY
				, X
				, ZERO
				, CTX))
		return ACLEXPR_CARVE_NO;

	if(V.m_num > 1) {
		aclspv_id_t ONES;
		aclspv_id_t ZERO_RES;

		ae2f_ifezerr(ZERO_RES = aclval_cfg_nil(TY, CTX))
			return ACLEXPR_CARVE_NO;

		/** all bits set, of the result type. */
		ae2f_ifezerr(ONES = aclcur_cfg_op1(
					0
					, SpvOpNot
					, TY
					, ZERO_RES
					, CTX))
			return ACLEXPR_CARVE_NO;

		ae2f_ifezerr(ID = aclcur_cfg_op3(
					0
					, SpvOpSelect
					, TY
					, ID
					, ONES
					, ZERO_RES
					, CTX))
			return ACLEXPR_CARVE_NO;

		*rdwr_prv_idval = ID;
		rdwr_prv_extra->m_type_original = TY;
		rdwr_prv_extra->m_mask_literal = 0;
		rdwr_prv_extra->m_literal.m_api_uintmax = 0;
		*r_ltrl = rdwr_prv_extra->m_literal;

		return ACLEXPR_CARVE_OK;
	}

	/** scalar: the comparison is bool; the use site casts to int. */
	*rdwr_prv_idval = ID;
	rdwr_prv_extra->m_type_original = BOOL_TY;
	rdwr_prv_extra->m_mask_literal = 0;
	rdwr_prv_extra->m_literal.m_api_uintmax = 0;
	*r_ltrl = rdwr_prv_extra->m_literal;

	return ACLEXPR_CARVE_OK;
}

/** `~x`: OpNot is bitwise and keeps the operand's (promoted) type; it is
 * elementwise for vectors. */
aclspv_bool_t aclexpr_carve1_bitnot(
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
	aclspv_id_t		ID;

	ae2f_ifezerr(TY) return ACLEXPR_CARVE_NO;
	ae2f_ifezerr(X)  return ACLEXPR_CARVE_NO;

	if(aclexpr_carve_cast_to_ty(TY, &X, &rdwr_lst_extra[0], CTX))
		return ACLEXPR_CARVE_NO;

	ae2f_ifezerr(ID = aclcur_cfg_op1(
				0
				, SpvOpNot
				, TY
				, X
				, CTX))
		return ACLEXPR_CARVE_NO;

	*rdwr_prv_idval = ID;
	rdwr_prv_extra->m_type_original = TY;
	rdwr_prv_extra->m_mask_literal = 0;
	rdwr_prv_extra->m_literal.m_api_uintmax = 0;
	*r_ltrl = rdwr_prv_extra->m_literal;

	return ACLEXPR_CARVE_OK;
}
