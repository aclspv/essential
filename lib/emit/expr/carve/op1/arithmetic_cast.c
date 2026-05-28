#include "./arithmetic_cast.h"
#include <emit/expr/carve/util.h>
#include <util/op.h>

#define CHECK unless(rdwr_lst_scale_buf[UTIL_OPBINCFG_OPCODE] == (aclspv_wrd_t)UTIL_OPBIN1_EXPLICIT_CAST) return c_lbl_continue

aclexpr_carve_lbl_t aclexpr_carve_op1_arithmetic_cast(
		struct aclexpr_visit_data* const ae2f_restrict	rdwr_data,
		aclspv_wrd_t* const ae2f_restrict rdwr_prv_idval,
		util_opbincfg_extra* const ae2f_restrict rdwr_prv_extra,
		ae2f_unused h_scale_t			h_lst_scale,
		aclspv_wrd_t* const ae2f_restrict	rdwr_lst_scale_buf,
		util_opbincfg_extra* ae2f_restrict rdwr_lst_extra,
		util_literal* ae2f_restrict	r_ltrl,
		aclexpr_carve_lbl_t		c_lbl_continue,
		aclexpr_carve_lbl_t		c_lbl_success,
		aclexpr_carve_lbl_t		c_lbl_err
		) { CHECK; {
	rdwr_lst_scale_buf[UTIL_OPBINCFG_MASKLITERAL] = rdwr_lst_extra[0].m_mask_literal;
	*rdwr_prv_idval = rdwr_lst_scale_buf[UTIL_OPBINCFG_END];
	if(rdwr_lst_scale_buf[UTIL_OPBINCFG_RESTYID] != rdwr_lst_extra[0].m_type_original) {
		const util_literal OLD_LITERAL = rdwr_lst_extra[0].m_literal;
		const aclspv_id_t OLD_ID = rdwr_lst_scale_buf[UTIL_OPBINCFG_END];

		dbg_prefix((Yup, old id is needed));
		dbg_call(fprintf, (stderr, "OLD_ID: %u\n", OLD_ID));

		dbg_prefix((Yup, new type id is needed));
		dbg_call(fprintf, (stderr, "NEW_TYPE: %u\n", rdwr_lst_scale_buf[UTIL_OPBINCFG_RESTYID]));

		switch(aclexpr_arithmetic_cast(
					OLD_ID
					, rdwr_lst_extra[0].m_type_original
					, rdwr_lst_extra[0].m_mask_literal
					, OLD_LITERAL
					, rdwr_prv_idval
					, rdwr_lst_scale_buf[UTIL_OPBINCFG_RESTYID]
					, &rdwr_prv_extra->m_literal
					, CTX(rdwr_data)
					)) {
			default:
			case EMIT_EXPR_NOT_THE_CASE:
			case EMIT_EXPR_FAILURE:
				return c_lbl_err;

			case EMIT_EXPR_SUCCESS:
				rdwr_lst_scale_buf[UTIL_OPBINCFG_MASKLITERAL] &= 0;
				ae2f_fallthrough;
			case EMIT_EXPR_SUCCESS_CONSTANT:
				rdwr_lst_scale_buf[UTIL_OPBINCFG_MASKLITERAL] &= 1;
				ae2f_fallthrough;
			case EMIT_EXPR_SUCCESS_LITERAL:
				break;
		}
	}

	assert(*rdwr_prv_idval);

	rdwr_prv_extra->m_mask_literal
		&= rdwr_lst_scale_buf[UTIL_OPBINCFG_MASKLITERAL];
	rdwr_prv_extra->m_type_original
		= rdwr_lst_scale_buf[UTIL_OPBINCFG_RESTYID];
	*r_ltrl = rdwr_prv_extra->m_literal;
	return c_lbl_success;
}}
