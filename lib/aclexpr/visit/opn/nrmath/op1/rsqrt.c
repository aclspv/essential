#include "./rsqrt.h"

#if	__ACLSPV_MATH_NR

#include <util/scale.h>
#include <util/wrdemit.h>
#include <aclexpr/op.h>

enum S_RET_ aclexpr_visit_op1_rsqrtf_nr(
		const CXCursor c_cur,
		ae2f_unused const h_aclspv_ctx_t			h_ctx,
		x_aclspv_vec* ae2f_restrict const h_cmdscale
		) {
	x_scale* ae2f_restrict	NEW_SCALE;
	aclspv_wrd_t* ae2f_restrict VEC;

	ae2f_ifnezerr_strict(clang_Cursor_getNumArguments(c_cur) != 1)
		return S_RET_FAIL;

	NEW_SCALE = get_last_scale_from_vec(h_cmdscale);
	NEW_SCALE = mk_scale_from_vec(
			h_cmdscale
			, 
			aclutil_count_to_sz(ACLEXPR_OPBINCFG_END + 1)
			+ (sizeof(aclexpr_opbincfg_extra))
			);

	ae2f_ifezerr(NEW_SCALE) {
		return S_RET_FAIL;
	}

	VEC = get_buf_from_scale2(aclspv_wrd_t, h_cmdscale, *NEW_SCALE);

	ae2f_ifezerr(VEC)
		return S_RET_FAIL;

	VEC[ACLEXPR_OPBINCFG_NUMOPRND]	= 1;				/** number of operands */
	VEC[ACLEXPR_OPBINCFG_OPCODE]	= ACLEXPR_OP1_RSQRTF_NR;
	VEC[ACLEXPR_OPBINCFG_RESTYID]	= 0;

	aclmemzero(&VEC[ACLEXPR_OPBINCFG_END + 1], sizeof(aclexpr_opbincfg_extra));

	return S_RET_GOOD;
}

#endif
