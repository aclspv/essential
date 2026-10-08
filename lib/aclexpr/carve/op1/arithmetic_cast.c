#include "./arithmetic_cast.h"
#include <aclexpr/carve/util.h>
#include <aclexpr/op.h>
#include <util/log.h>

#define CHECK unless(rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_OPCODE] == (aclspv_wrd_t)ACLEXPR_OP1_EXPLICIT_CAST) return c_lbl_continue

aclexpr_carve_lbl_t aclexpr_carve1_arithmetic_cast(
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
		) { CHECK; {
	rdwr_prv_extra->m_mask_literal = (aclspv_wrd_t)ACLUTIL_LITERAL_LITERAL;

	*rdwr_prv_idval = rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_END];
	if(rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_RESTYID] != rdwr_lst_extra[0].m_type_original) {
		const aclutil_literal OLD_LITERAL = rdwr_lst_extra[0].m_literal;
		const aclspv_id_t OLD_ID = rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_END];

		dbg_prefix(("Yup, old id is needed"));
		dbg_call(fprintf, (stderr, "OLD_ID: %u\n", OLD_ID));

		dbg_prefix(("Yup, new type id is needed"));
		dbg_call(fprintf, (stderr, "NEW_TYPE: %u\n", rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_RESTYID]));

		switch(aclexpr_arithmetic_cast(
					OLD_ID
					, rdwr_lst_extra->m_type_original
					, rdwr_lst_extra->m_mask_literal
					, OLD_LITERAL
					, rdwr_prv_idval
					, rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_RESTYID]
					, &rdwr_prv_extra->m_literal
					, CTX(rdwr_data)
					)) {
			default:
			case ACLEXPR_RET_NOT_THE_CASE:
			case ACLEXPR_RET_FAILURE:
				return c_lbl_err;

			case ACLEXPR_RET_SUCCESS:
				rdwr_prv_extra->m_mask_literal &= 0;
				ae2f_fallthrough;
			case ACLEXPR_RET_SUCCESS_CONSTANT:
				rdwr_prv_extra->m_mask_literal &= 1;
				ae2f_fallthrough;
			case ACLEXPR_RET_SUCCESS_LITERAL:
				break;
		}
	} else {
		/** same-type no-op cast: literality and folded value pass through
		 * unchanged. the LITERAL pre-set above must not leak into this
		 * path — a runtime value must stay runtime, and a literal must
		 * keep its value rather than the visitor's memset-0. */
		rdwr_prv_extra->m_mask_literal = rdwr_lst_extra[0].m_mask_literal;
		rdwr_prv_extra->m_literal = rdwr_lst_extra[0].m_literal;
	}

	assert(*rdwr_prv_idval);

	rdwr_prv_extra->m_type_original
		= rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_RESTYID];
	*r_ltrl = rdwr_prv_extra->m_literal;
	return c_lbl_success;
}}
