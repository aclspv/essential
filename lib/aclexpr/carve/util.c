#include "./util.h"

/** cast the RHS value to `c_ty`.
 *
 * the store layer deliberately enforces exact type equality
 * (`aclcur_store_reg`), so an assignment `lhs = rhs` must materialise the
 * implicit C conversion first. this is the single place that does it for
 * the write carves.
 *
 * returns 0 on success, nonzero when the conversion is not possible. */
aclspv_bool_t aclexpr_carve_cast_to_ty(
		aclspv_id_t			c_ty,
		aclspv_id_t* ae2f_restrict	rw_val,
		aclexpr_opbincfg_extra* ae2f_restrict rw_extra,
		h_aclspv_ctx_t			h_ctx
		) {
	enum ACLEXPR_RET_	RET;

	if(rw_extra->m_type_original == c_ty)
		return 0;

	RET = aclexpr_arithmetic_cast(
			*rw_val
			, rw_extra->m_type_original
			, rw_extra->m_mask_literal
			, rw_extra->m_literal
			, rw_val
			, c_ty
			, &rw_extra->m_literal
			, h_ctx);

	switch(RET) {
		case ACLEXPR_RET_FAILURE:
		case ACLEXPR_RET_NOT_THE_CASE:
		default:
			return 1;

		case ACLEXPR_RET_SUCCESS_LITERAL:
			rw_extra->m_mask_literal = ACLUTIL_LTRL_MASK_LTRL;
			break;

		case ACLEXPR_RET_SUCCESS_CONSTANT:
			rw_extra->m_mask_literal = ACLUTIL_LTRL_MASK_CONST;
			break;

		case ACLEXPR_RET_SUCCESS:
			rw_extra->m_mask_literal = ACLUTIL_LTRL_RT;
			break;
	}

	rw_extra->m_type_original = c_ty;
	return 0;
}

/** cast the RHS value to the storage type of `c_reg`. */
aclspv_bool_t aclexpr_carve_cast_to_reg(
		aclcur_regid_t			c_reg,
		aclspv_id_t* ae2f_restrict	rw_val,
		aclexpr_opbincfg_extra* ae2f_restrict rw_extra,
		h_aclspv_ctx_t			h_ctx
		) {
	h_aclcur_t p;

	c_reg = aclcur_reg_resolve(c_reg, h_ctx);
	ae2f_ifezerr(c_reg)				return 1;
	ae2f_ifezerr(p = aclcur_get_node(c_reg, h_ctx))	return 1;

	return aclexpr_carve_cast_to_ty(p->m_ty, rw_val, rw_extra, h_ctx);
}
