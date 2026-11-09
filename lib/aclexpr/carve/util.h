#ifndef	aclexpr_carve_util_h
#define	aclexpr_carve_util_h

#include <aclexpr.h>
#include <aclexpr/op.h>
#include <aclcur/common.h>

/** cast the RHS value to an explicit type id.
 * returns 0 on success, nonzero when the conversion is not possible. */
aclspv_bool_t aclexpr_carve_cast_to_ty(
		aclspv_id_t			c_ty,
		aclspv_id_t* ae2f_restrict	rw_val,
		aclexpr_opbincfg_extra* ae2f_restrict rw_extra,
		h_aclspv_ctx_t			h_ctx
		);

/** cast the RHS value to the storage type of `c_reg`.
 * returns 0 on success, nonzero when the conversion is not possible. */
aclspv_bool_t aclexpr_carve_cast_to_reg(
		aclcur_regid_t			c_reg,
		aclspv_id_t* ae2f_restrict	rw_val,
		aclexpr_opbincfg_extra* ae2f_restrict rw_extra,
		h_aclspv_ctx_t			h_ctx
		);

#define CTX(rdwr_data)			(rdwr_data)->m_ctx
#define EXPRSTCK_COUNT(rdwr_data)	(rdwr_data)->m_exprcount
#define	RET_LITERAL_REF(rdwr_data)	(rdwr_data)->m_ret_literal

#define	CMDSTCK_SCALE(CTX)	(CTX)->m_tmp.m_v1
#define	CMDCOUNTERSTCK(CTX)	(CTX)->m_tmp.m_v2

#endif
