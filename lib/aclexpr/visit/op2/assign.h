#ifndef	aclexpr_visit_op2_assign_h
#define	aclexpr_visit_op2_assign_h

#include <util/constant.h>
#include <util/scale.h>
#include <util/cxtp.h>
#include <util/wrdemit.h>

#include <aclexpr.h>
#include <aclexpr/visit/api.h>
#include <aclexpr/op.h>

#include <ae2f/cc/memset.h>

ae2f_inline_f static aclexpr_visit_lbl_t aclexpr_visit_op2_assign(
		const CXCursor				c_cur,
		const h_aclspv_ctx_t			h_ctx, 
		x_aclspv_vec* ae2f_restrict const	h_cmdscale,
		aclexpr_visit_lbl_t			c_lbl_success,
		aclexpr_visit_lbl_t			c_lbl_fail,
		aclexpr_visit_lbl_t			c_lbl_pass
		) {
	x_scale* ae2f_restrict		NEW_SCALE;
	aclspv_wrd_t* ae2f_restrict	VEC;

	ae2fpat_assertunless(h_ctx) {
		return c_lbl_fail;
	}

	unless(CXCursor_BinaryOperator == c_cur.kind && CXBinaryOperator_Assign == clang_getCursorBinaryOperatorKind(c_cur))
		return c_lbl_pass;


	NEW_SCALE = get_last_scale_from_vec(h_cmdscale);
	NEW_SCALE = mk_scale_from_vec(
			h_cmdscale
			, 
			aclutil_count_to_sz(ACLEXPR_OPBINCFG_END + 2)
			+ (sizeof(aclexpr_opbincfg_extra) * 2)
			);
	ae2f_expected_but_else(NEW_SCALE)
		return c_lbl_fail;

	VEC = get_buf_from_scale2(aclspv_wrd_t, h_cmdscale, *NEW_SCALE);

	ae2f_expected_but_else(VEC)
		return c_lbl_fail;

	VEC[ACLEXPR_OPBINCFG_NUMOPRND]		= 2;
	VEC[ACLEXPR_OPBINCFG_OPCODE]		= ACLEXPR_OP2_ASSIGN;

	ae2fcc_memset(&VEC[ACLEXPR_OPBINCFG_END + 2], 0, sizeof(aclexpr_opbincfg_extra) << 1);

	((aclexpr_opbincfg_extra* ae2f_restrict)(void*)&VEC[ACLEXPR_OPBINCFG_END + 2])->m_state0 = ACLEXPR_OPCFGX_STATE0_VAL_NO_NEED;

	return c_lbl_success;
}

#endif
