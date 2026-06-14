#include <util/scale.h>

#include "./op.h"
#include "./beforevisit.h"
#include "./beforevisit/util.h"
#include "./beforevisit/bool1.h"

aclspv_bool_t aclexpr_beforevisit(
		struct aclexpr_visit_data* const ae2f_restrict	rdwr_data,
		aclspv_wrd_t					c_opidx
		) {
	h_scale_t LST_SCALE				= get_last_scale_from_vec(&CMDSTCK_SCALE(rdwr_data->m_ctx));
	aclspv_wrd_t* ae2f_restrict LST_SCALE_BUF	= get_buf_from_scale2(
			aclspv_wrd_t
			, &CMDSTCK_SCALE(CTX(rdwr_data))
			, *LST_SCALE
			);

	switch(LST_SCALE_BUF[ACLEXPR_OPBINCFG_OPCODE]) {
		case ACLEXPR_OP2_AND_L:
		case ACLEXPR_OP2_OR_L:
			return aclexpr_beforevisit2_bool1(
					rdwr_data
					, c_opidx
					);

		default:
			return 0;
	}

	ae2f_unreachable();
}
