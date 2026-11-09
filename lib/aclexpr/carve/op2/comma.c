#include "./comma.h"

#include <aclexpr/carve/util.h>

/** `a, b`: both operands have already been carved (the left for its side
 * effects). the value is the right operand; no instruction of our own.
 *
 * NOPURE is OR-propagated across both operands by the visit loop, so an
 * impure left keeps the whole expression impure. */
aclspv_bool_t aclexpr_carve2_comma(
		struct aclexpr_visit_data	ae2fcc_arg_sarr_restrict(rdwr_data, 1),
		aclspv_wrd_t			ae2fcc_arg_sarr_restrict(rdwr_prv_idval, 1),
		aclexpr_opbincfg_extra		ae2fcc_arg_sarr_restrict(rdwr_prv_extra, 1),
		aclspv_wrd_t			ae2fcc_arg_sarr_restrict(rdwr_lst_scale_buf, ACLEXPR_OPBINCFG_END + 2),
		aclexpr_opbincfg_extra		ae2fcc_arg_sarr_restrict(rdwr_lst_extra, 2),
		aclutil_literal			ae2fcc_arg_sarr_restrict(r_ltrl, 1)
		) {
	const h_aclspv_ctx_t	CTX = rdwr_data->m_ctx;
	const aclspv_id_t	TY = rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_RESTYID];
	aclspv_id_t		R = rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_END + 1];

	ae2f_ifezerr(TY) return ACLEXPR_CARVE_NO;
	ae2f_ifezerr(R)  return ACLEXPR_CARVE_NO;

	/* the comma's type is the right operand's type; this is normally a
	 * no-op but keeps the contract with aclcur_store_reg. */
	if(aclexpr_carve_cast_to_ty(TY, &R, &rdwr_lst_extra[1], CTX))
		return ACLEXPR_CARVE_NO;

	*rdwr_prv_idval = R;
	*rdwr_prv_extra = rdwr_lst_extra[1];
	rdwr_prv_extra->m_type_original = TY;

	(void)r_ltrl;
	return ACLEXPR_CARVE_OK;
}
