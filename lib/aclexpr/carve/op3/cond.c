#include "./cond.h"

#include <aclexpr/carve/util.h>

#include <aclcur/common.h>

/** `c ? t : f` fast lane: both arms are pure, so evaluating both is
 * observationally identical to selecting one. */
static aclspv_bool_t s_cond_select(
		struct aclexpr_visit_data* ae2f_restrict const	rdwr_data,
		aclspv_wrd_t* ae2f_restrict const		rdwr_prv_idval,
		aclexpr_opbincfg_extra* ae2f_restrict const	rdwr_prv_extra,
		aclspv_wrd_t* ae2f_restrict const		rdwr_lst_scale_buf,
		aclexpr_opbincfg_extra* ae2f_restrict const	rdwr_lst_extra,
		aclutil_literal* ae2f_restrict const		r_ltrl
		) {
	const h_aclspv_ctx_t	CTX = rdwr_data->m_ctx;
	const aclspv_id_t	TY = rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_RESTYID];
	aclspv_id_t		COND = rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_END];
	aclspv_id_t		T = rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_END + 1];
	aclspv_id_t		F = rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_END + 2];
	aclspv_id_t		ID;
	struct aclutil_cxtp_veccfg_t V;

	if(aclexpr_carve_cast_to_ty(ACLSPV_ID_BOOL, &COND, &rdwr_lst_extra[0], CTX))
		return ACLEXPR_CARVE_NO;

	/** OpSelect needs a bool vector matching the result vector's shape.
	 * the C condition is scalar, so broadcast the bool scalar with
	 * OpCompositeConstruct. */
	V = aclty_get_veccfg(TY, CTX);
	if(V.m_num > 1) {
		aclspv_vid_t	CONDS[__ACLSPV_MAX_OPRND];
		aclspv_wrd_t	i;

		if(V.m_num > __ACLSPV_MAX_OPRND)
			return ACLEXPR_CARVE_NO;

		for(i = 0; i < V.m_num; ++i)
			CONDS[i] = COND;

		ae2f_ifezerr(COND = aclcur_cfg_opx(
					0
					, SpvOpCompositeConstruct
					, aclty_simd_chxg_sclr(TY, ACLSPV_ID_BOOL, CTX)
					, CONDS
					, V.m_num
					, CTX))
			return ACLEXPR_CARVE_NO;
	}

	if(aclexpr_carve_cast_to_ty(TY, &T, &rdwr_lst_extra[1], CTX))
		return ACLEXPR_CARVE_NO;

	if(aclexpr_carve_cast_to_ty(TY, &F, &rdwr_lst_extra[2], CTX))
		return ACLEXPR_CARVE_NO;

	ae2f_ifezerr(ID = aclcur_cfg_op3(
				0
				, SpvOpSelect
				, TY
				, COND
				, T
				, F
				, CTX))
		return ACLEXPR_CARVE_NO;

	*rdwr_prv_idval = ID;
	rdwr_prv_extra->m_type_original = TY;
	rdwr_prv_extra->m_mask_literal = 0;
	rdwr_prv_extra->m_literal.m_api_uintmax = 0;
	*r_ltrl = rdwr_prv_extra->m_literal;

	return ACLEXPR_CARVE_OK;
}

/** `c ? t : f` branch lane: at least one arm has a side effect, so the
 * unselected arm must not run. the before-visitor already emitted the
 * branch and stored the true arm; finish the false arm, join, and load the
 * temp register with a phi over both arm blocks. */
static aclspv_bool_t s_cond_branch(
		struct aclexpr_visit_data* ae2f_restrict const	rdwr_data,
		aclspv_wrd_t* ae2f_restrict const		rdwr_prv_idval,
		aclexpr_opbincfg_extra* ae2f_restrict const	rdwr_prv_extra,
		aclspv_wrd_t* ae2f_restrict const		rdwr_lst_scale_buf,
		aclexpr_opbincfg_extra* ae2f_restrict const	rdwr_lst_extra,
		aclutil_literal* ae2f_restrict const		r_ltrl
		) {
	const h_aclspv_ctx_t	CTX = rdwr_data->m_ctx;
	const aclspv_id_t	TY = rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_RESTYID];
	const aclspv_id_t	REG = rdwr_lst_extra[0].m_extra.m_cond.m_reg;
	const aclspv_id_t	L_TRUE = rdwr_lst_extra[0].m_extra.m_cond.m_id_true;
	const aclspv_id_t	L_FALSE = rdwr_lst_extra[0].m_extra.m_cond.m_id_false;
	const aclspv_id_t	L_MERGE = rdwr_lst_extra[0].m_extra.m_cond.m_id_merge;
	aclspv_id_t		F = rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_END + 2];
	aclspv_id_t		ID;

	ae2f_ifezerr(REG)	return ACLEXPR_CARVE_NO;
	ae2f_ifezerr(L_TRUE)	return ACLEXPR_CARVE_NO;
	ae2f_ifezerr(L_FALSE)	return ACLEXPR_CARVE_NO;
	ae2f_ifezerr(L_MERGE)	return ACLEXPR_CARVE_NO;

	if(aclexpr_carve_cast_to_ty(TY, &F, &rdwr_lst_extra[2], CTX))
		return ACLEXPR_CARVE_NO;

	rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_END + 2] = F;

	/** false arm's value lands in the false block. */
	ae2f_ifezerr(aclcur_store_reg(REG, F, CTX))
		return ACLEXPR_CARVE_NO;

	/** close the false block and join. */
	ae2f_ifezerr(aclcur_cfg_jmp(L_MERGE, CTX))
		return ACLEXPR_CARVE_NO;

	ae2f_ifezerr(aclcur_cfg_lbl(L_MERGE, CTX))
		return ACLEXPR_CARVE_NO;

	ae2f_ifezerr(ID = aclcur_load_reg(REG, CTX, 0))
		return ACLEXPR_CARVE_NO;

	/** an edge per arm block; aclcur_cfg_load_phi is idempotent by label,
	 * so an edge the load's own dfs already attached is skipped. */
	ae2f_ifnezerr_strict(aclcur_cfg_load_phi(ID, L_TRUE, CTX))
		return ACLEXPR_CARVE_NO;
	ae2f_ifnezerr_strict(aclcur_cfg_load_phi(ID, L_FALSE, CTX))
		return ACLEXPR_CARVE_NO;

	*rdwr_prv_idval = ID;
	rdwr_prv_extra->m_type_original = TY;
	rdwr_prv_extra->m_mask_literal = 0;
	rdwr_prv_extra->m_literal.m_api_uintmax = 0;
	*r_ltrl = rdwr_prv_extra->m_literal;

	return ACLEXPR_CARVE_OK;
}

aclspv_bool_t aclexpr_carve3_cond(
		struct aclexpr_visit_data	ae2fcc_arg_sarr_restrict(rdwr_data, 1),
		aclspv_wrd_t			ae2fcc_arg_sarr_restrict(rdwr_prv_idval, 1),
		aclexpr_opbincfg_extra		ae2fcc_arg_sarr_restrict(rdwr_prv_extra, 1),
		aclspv_wrd_t			ae2fcc_arg_sarr_restrict(rdwr_lst_scale_buf, ACLEXPR_OPBINCFG_END + 3),
		aclexpr_opbincfg_extra		ae2fcc_arg_sarr_restrict(rdwr_lst_extra, 3),
		aclutil_literal			ae2fcc_arg_sarr_restrict(r_ltrl, 1)
		) {
	const aclspv_id_t	TY = rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_RESTYID];

	ae2f_ifezerr(TY)	return ACLEXPR_CARVE_NO;
	ae2f_ifezerr(rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_END])
		return ACLEXPR_CARVE_NO;
	ae2f_ifezerr(rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_END + 1])
		return ACLEXPR_CARVE_NO;
	ae2f_ifezerr(rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_END + 2])
		return ACLEXPR_CARVE_NO;

	if(rdwr_lst_extra[0].m_state0
			& (aclspv_wrd_t)ACLEXPR_OPCFGX_STATE0_COND_BRANCH)
		return s_cond_branch(
				rdwr_data
				, rdwr_prv_idval
				, rdwr_prv_extra
				, rdwr_lst_scale_buf
				, rdwr_lst_extra
				, r_ltrl
				);

	return s_cond_select(
			rdwr_data
			, rdwr_prv_idval
			, rdwr_prv_extra
			, rdwr_lst_scale_buf
			, rdwr_lst_extra
			, r_ltrl
			);
}
