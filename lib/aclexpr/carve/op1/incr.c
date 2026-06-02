#include "./incr.h"

#if	0

ae2f_inline_f	ae2f_ccconst static aclspv_bool_t	s_check(aclspv_wrd_t	c_op) {
	switch(c_op) {
		case ACLEXPR_OP1_POSTDEG:
		case ACLEXPR_OP1_POSTINC:
		case ACLEXPR_OP1_PREDEG:
		case ACLEXPR_OP1_PREINC:
			return 1;

		default:
			return 0;
	}
}

aclexpr_carve_lbl_t aclexpr_carve_op1_incr(
		struct aclexpr_visit_data* const ae2f_restrict	rdwr_data,
		aclspv_wrd_t* const ae2f_restrict rdwr_prv_idval,
		aclexpr_opbincfg_extra* const ae2f_restrict rdwr_prv_extra,
		ae2f_unused h_scale_t			h_lst_scale,
		aclspv_wrd_t* const ae2f_restrict	rdwr_lst_scale_buf,
		aclexpr_opbincfg_extra* ae2f_restrict rdwr_lst_extra,
		aclutil_literal* ae2f_restrict	r_ltrl,
		aclexpr_carve_lbl_t		c_lbl_continue,
		aclexpr_carve_lbl_t		c_lbl_success,
		aclexpr_carve_lbl_t		c_lbl_err
		) { unless(s_check(rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_OPCODE])) return c_lbl_continue; {
	switch(rdwr_lst_extra->m_mask_literal) {
		case ACLUTIL_LITERAL_MASK_LITERAL:
		case ACLUTIL_LITERAL_LITERAL:


		case ACLUTIL_LITERAL_CONSTANT:
		case ACLUTIL_LITERAL_RT:


		default:
			ae2fpat_assertassume_strict(0);
			return c_lbl_err;
	}
} ae2f_unreachable(); }

#endif
