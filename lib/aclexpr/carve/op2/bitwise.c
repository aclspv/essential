#include "./bitwise.h"

static ae2f_ccconst aclspv_wrd_t	s_opcvrt(const aclspv_wrd_t c_op, const aclspv_bool_t c_is_logical) {
	switch(c_op) {
		case ACLEXPR_OP2_AND:
			return  ae2f_bll_sel_bool1(
					aclspv_wrd_t, c_is_logical
					, (aclspv_wrd_t)SpvOpLogicalAnd
					, (aclspv_wrd_t)SpvOpBitwiseAnd);

		case ACLEXPR_OP2_OR:
			return  ae2f_bll_sel_bool1(
					aclspv_wrd_t, c_is_logical
					, (aclspv_wrd_t)SpvOpLogicalOr
					, (aclspv_wrd_t)SpvOpBitwiseOr);

		case ACLEXPR_OP2_XOR:
			return  ae2f_bll_sel_bool1(
					aclspv_wrd_t, c_is_logical
					, (aclspv_wrd_t)SpvOpLogicalNotEqual
					, (aclspv_wrd_t)SpvOpBitwiseXor);

		default:
			return 0;
	}
}

ae2f_unused ae2f_inline_f static aclspv_wrd_t s_decide_cast(const enum ACLSPV_ID_ c_id0, const enum ACLSPV_ID_ c_id1, const h_aclspv_ctx_t h_ctx) {
	/** one is 0? */
	unless(c_id0 & c_id1)
		return ae2f_bll_sel_bool_naive1(B_id_default_t, c_id0, (B_id_default_t) c_id0, (B_id_default_t) c_id1);

	unless(c_id0 ^ c_id1)
		return (B_id_default_t) c_id0;

	unless(aclid_is_arithmetic((B_id_default_t) c_id0, h_ctx) & aclid_is_arithmetic((B_id_default_t) c_id1, h_ctx))
		return 0;

	if(aclutil_cxtp_get_veccfg((B_id_default_t) c_id0, h_ctx).m_num ^ aclutil_cxtp_get_veccfg((B_id_default_t) c_id1, h_ctx).m_num) {
		unless(aclutil_cxtp_get_veccfg((B_id_default_t) c_id0, h_ctx).m_num >> 1) {
			return (B_id_default_t) c_id1;
		} else unless(aclutil_cxtp_get_veccfg((B_id_default_t) c_id1, h_ctx).m_num >> 1) {
			return (B_id_default_t) c_id0;
		} return 0;
	}

	unless(aclid_is_vectorisable((B_id_default_t) c_id0)) {
		return ae2f_bll_sel_bool1(B_id_default_t, s_decide_cast(
				(enum ACLSPV_ID_) aclutil_cxtp_get_veccfg((B_id_default_t) c_id0, h_ctx).m_ty_elm
					, (enum ACLSPV_ID_) aclutil_cxtp_get_veccfg((B_id_default_t) c_id1, h_ctx).m_ty_elm
					, h_ctx
					), (B_id_default_t) c_id0, (B_id_default_t) c_id1);
	}

	unless(aclid_is_int((B_id_default_t) c_id0) & aclid_is_int((B_id_default_t) c_id1))
		return 0;

	return ae2f_bll_sel_bool1(aclspv_wrd_t, aclid_is_sint((aclspv_wrd_t)c_id0), (aclspv_wrd_t)c_id0, (aclspv_wrd_t)c_id1);
}

 aclexpr_carve_lbl_t aclexpr_carve_op2_bitwise(
		struct aclexpr_visit_data* const ae2f_restrict	rdwr_data,
		aclspv_wrd_t* const ae2f_restrict		rdwr_prv_idval,
		aclexpr_opbincfg_extra* const ae2f_restrict	rdwr_prv_extra,
		ae2f_unused h_scale_t				h_lst_scale,
		aclspv_wrd_t* const ae2f_restrict		rdwr_lst_scale_buf,
		aclexpr_opbincfg_extra* ae2f_restrict		rdwr_lst_extra,
		aclutil_literal* ae2f_restrict			r_ltrl,
		aclexpr_carve_lbl_t				c_lbl_continue,
		aclexpr_carve_lbl_t				c_lbl_success,
		aclexpr_carve_lbl_t				c_lbl_err
		) { aclspv_wrd_t OPNATIVE; ae2fcc_unless(OPNATIVE = s_opcvrt(rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_OPCODE], !!(rdwr_lst_extra->m_state0 & ACLEXPR_OPCFGX_STATE0_LOGICAL))) return c_lbl_continue; {
	 (void)rdwr_data;
	 (void)rdwr_prv_idval;
	 (void)rdwr_prv_extra;
	 (void)h_lst_scale;
	 (void)rdwr_lst_scale_buf;
	 (void)rdwr_lst_extra;
	 (void)r_ltrl;
	 (void)c_lbl_continue;
	 (void)c_lbl_success;
	 (void)c_lbl_err;

	 return c_lbl_err;
}}
