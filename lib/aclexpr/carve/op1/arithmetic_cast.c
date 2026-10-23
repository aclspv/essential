#include "./arithmetic_cast.h"
#include <aclexpr/carve/util.h>
#include <aclexpr/op.h>
#include <util/log.h>

	aclspv_bool_t aclexpr_carve1_arithmetic_cast(
			struct aclexpr_visit_data	ae2fcc_arg_sarr_restrict(rdwr_data, 1),
			aclspv_wrd_t			ae2fcc_arg_sarr_restrict(rdwr_prv_idval, 1),
			aclexpr_opbincfg_extra		ae2fcc_arg_sarr_restrict(rdwr_prv_extra, 1),
			aclspv_wrd_t			ae2fcc_arg_sarr_restrict(rdwr_lst_scale_buf, ACLEXPR_OPBINCFG_END + 1),
			aclexpr_opbincfg_extra		ae2fcc_arg_sarr_restrict(rdwr_lst_extra, 1),
			ae2f_unused aclutil_literal	ae2fcc_arg_sarr_restrict(r_ltrl, 1)
			) {
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
					err_puts("aclexpr_arithmetic_cast\n");
					return ACLEXPR_CARVE_NO;

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
		return ACLEXPR_CARVE_OK;
	}
