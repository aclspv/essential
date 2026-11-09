#include "./cond.h"
#include "./util.h"

#include <aclcur/common.h>
#include <aclcur/reg/var.h>

#include <aclexpr/op.h>
#include <aclexpr/sideeffect.h>
#include <aclexpr/carve/util.h>

#include <util/scale.h>

/** `c ? t : f` with at least one impure arm.
 *
 * the branch lane mirrors the bool1 short-circuit lane: the condition is
 * evaluated in the current block, which is terminated by a conditional
 * branch whose two targets are the arm blocks and whose merge is the join.
 * each arm stores its value into a function-local temp register; the merge
 * loads it with a phi over the two arm blocks.
 *
 * setup happens in two steps so each arm still returns its own operand:
 *
 *   idx 1 (true arm):   cast the condition, mint the temp reg, emit the
 *                       branch, open the true block;
 *   idx 2 (false arm):  store the true value, jump to the merge, open the
 *                       false block.
 *
 * the false store, the final jump and the merge load/phi happen in the
 * carve once the false arm's value exists. the shared state rides on the
 * condition extra under ACLEXPR_OPCFGX_STATE0_COND_BRANCH.
 * */
static aclspv_bool_t s_cond_open_true(
		aclspv_wrd_t* const ae2f_restrict		LST_SCALE_BUF,
		aclexpr_opbincfg_extra* const ae2f_restrict	LST_EXTRA,
		struct aclexpr_visit_data* const ae2f_restrict	rdwr_data
		) {
	const h_aclspv_ctx_t	CTX = rdwr_data->m_ctx;
	const aclspv_id_t	TY = LST_SCALE_BUF[ACLEXPR_OPBINCFG_RESTYID];
	aclspv_id_t		COND = LST_SCALE_BUF[ACLEXPR_OPBINCFG_END];
	aclspv_id_t		REG;
	aclspv_id_t		L_TRUE;
	aclspv_id_t		L_FALSE;
	aclspv_id_t		L_MERGE;
	const aclcur_reg_t	REGIN = aclcur_regvar_in();

	/** the branch needs a scalar bool condition. */
	if(aclexpr_carve_cast_to_ty(ACLSPV_ID_BOOL, &COND, &LST_EXTRA[0], CTX))
		return 1;

	LST_SCALE_BUF[ACLEXPR_OPBINCFG_END] = COND;

	/** temp register of the ternary's own result type. */
	ae2f_ifezerr(REG = aclcur_cfg_reg(
				clang_getNullCursor()
				, TY
				, SpvStorageClassFunction
				, &REGIN
				, CTX))
		return 1;

	/** labels are forward references: created before the branch, opened as
	 * the traversal reaches them. */
	L_TRUE	= CTX->m_id++;
	L_FALSE	= CTX->m_id++;
	L_MERGE	= CTX->m_id++;

	LST_EXTRA[0].m_state0
		|= (aclspv_wrd_t)ACLEXPR_OPCFGX_STATE0_COND_BRANCH;
	LST_EXTRA[0].m_extra.m_cond.m_reg	= REG;
	LST_EXTRA[0].m_extra.m_cond.m_id_true	= L_TRUE;
	LST_EXTRA[0].m_extra.m_cond.m_id_false	= L_FALSE;
	LST_EXTRA[0].m_extra.m_cond.m_id_merge	= L_MERGE;

	/** terminate the condition block: select an arm. */
	ae2f_ifezerr(aclcur_cfg_jc(
				COND
				, L_TRUE
				, L_FALSE
				, 0, 0
				, L_MERGE, 0, 0
				, CTX))
		return 1;

	/** the true arm evaluates here. */
	ae2f_ifezerr(aclcur_cfg_lbl(L_TRUE, CTX))
		return 1;

	return 0;
}

/** the true arm's value exists: store it and jump to the merge. */
static aclspv_bool_t s_cond_close_true(
		aclspv_wrd_t* const ae2f_restrict		LST_SCALE_BUF,
		aclexpr_opbincfg_extra* const ae2f_restrict	LST_EXTRA,
		struct aclexpr_visit_data* const ae2f_restrict	rdwr_data
		) {
	const h_aclspv_ctx_t	CTX = rdwr_data->m_ctx;
	const aclspv_id_t	TY = LST_SCALE_BUF[ACLEXPR_OPBINCFG_RESTYID];
	aclspv_id_t		T = LST_SCALE_BUF[ACLEXPR_OPBINCFG_END + 1];

	if(aclexpr_carve_cast_to_ty(TY, &T, &LST_EXTRA[1], CTX))
		return 1;

	LST_SCALE_BUF[ACLEXPR_OPBINCFG_END + 1] = T;

	ae2f_ifezerr(aclcur_store_reg(
				LST_EXTRA[0].m_extra.m_cond.m_reg
				, T
				, CTX))
		return 1;

	ae2f_ifezerr(aclcur_cfg_jmp(
				LST_EXTRA[0].m_extra.m_cond.m_id_merge
				, CTX))
		return 1;

	/** the false arm evaluates here. */
	ae2f_ifezerr(aclcur_cfg_lbl(
				LST_EXTRA[0].m_extra.m_cond.m_id_false
				, CTX))
		return 1;

	return 0;
}

aclspv_bool_t aclexpr_beforevisit3_cond(
		CXCursor					c_cur,
		struct aclexpr_visit_data* const ae2f_restrict	rdwr_data,
		aclspv_wrd_t					c_opidx
		) {
	h_scale_t					LST_SCALE;
	aclspv_wrd_t* ae2f_restrict			LST_SCALE_BUF;
	aclexpr_opbincfg_extra* ae2f_restrict		LST_EXTRA;

	ae2f_expected_but_else(rdwr_data)	return 1;

	LST_SCALE = get_last_scale_from_vec(&CMDSTCK_SCALE(rdwr_data->m_ctx));
	ae2f_expected_but_else(LST_SCALE)	return 1;

	LST_SCALE_BUF = get_buf_from_scale2(
			aclspv_wrd_t
			, &CMDSTCK_SCALE(rdwr_data->m_ctx)
			, *LST_SCALE);
	ae2f_expected_but_else(LST_SCALE_BUF)	return 1;

	LST_EXTRA = (void* ae2f_restrict)(
			LST_SCALE_BUF
			+ LST_SCALE_BUF[ACLEXPR_OPBINCFG_NUMOPRND]
			+ ACLEXPR_OPBINCFG_END);

	switch(c_opidx) {
		default:
			return 0;

		case 1:
			/** already lowered: an inner cursor of the true arm (a
			 * parenthesised arm re-enters at the same index). */
			if(LST_EXTRA[0].m_state0
					& (aclspv_wrd_t)ACLEXPR_OPCFGX_STATE0_COND_BRANCH)
				return 0;

			/** only the arms decide. the condition's own side effect is
			 * harmless: it runs once either way. */
			if(!s_cursor_impure(c_cur)
					&& !clang_Cursor_isNull(LST_EXTRA[2].m_cursor)
					&& !s_cursor_impure(LST_EXTRA[2].m_cursor))
				return 0;

			return s_cond_open_true(LST_SCALE_BUF, LST_EXTRA, rdwr_data);

		case 2:
			if(!(LST_EXTRA[0].m_state0
						& (aclspv_wrd_t)ACLEXPR_OPCFGX_STATE0_COND_BRANCH))
				return 0;

			/** an inner cursor of the false arm re-enters at the same
			 * index; the false block is already open. */
			if(aclcur_get_lbl_cur(rdwr_data->m_ctx)
					== LST_EXTRA[0].m_extra.m_cond.m_id_false)
				return 0;

			return s_cond_close_true(LST_SCALE_BUF, LST_EXTRA, rdwr_data);
	}
}
