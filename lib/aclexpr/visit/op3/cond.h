#ifndef	aclexpr_visit_op3_cond_h
#define	aclexpr_visit_op3_cond_h

#include "../../op.h"
#include "../api.h"

#include <util/scale.h>
#include <aclutil/wrdemit.h>

#include <aclty.h>
#include <aclexpr.h>
#include <aclmem.h>
#include <aclattr/getkinder.h>

/** `c ? t : f` — three operands: condition, true value, false value.
 *
 * the three operand cursors are stashed on the extras as well: the
 * before-visitor decides the branch lane at the true arm, when the false
 * arm has not been visited yet, so it must be able to look the false arm
 * up statically. */
ae2f_inline_f static aclexpr_visit_lbl_t aclexpr_visit_op3_cond(
		const CXCursor				c_cur,
		const h_aclspv_ctx_t			h_ctx,
		x_aclspv_vec* ae2f_restrict const	h_cmdscale,
		aclexpr_visit_lbl_t			c_lbl_success,
		aclexpr_visit_lbl_t			c_lbl_fail,
		aclexpr_visit_lbl_t			c_lbl_pass
		) {
	x_scale* ae2f_restrict		NEW_SCALE;
	aclspv_wrd_t* ae2f_restrict	VEC;
	aclexpr_opbincfg_extra* ae2f_restrict EXTRAS;
	aclspv_id_t			TY;
	CXCursor			CH[3];
	struct aclattr_getkinder	GK;

	if(c_cur.kind != CXCursor_ConditionalOperator)
		return c_lbl_pass;

	TY = aclutil_mk_cxtp_by_type2(
			clang_getCanonicalType(clang_getCursorType(c_cur))
			, h_ctx);
	ae2f_ifezerr(TY) return c_lbl_fail;

	NEW_SCALE = mk_scale_from_vec(
			h_cmdscale
			, aclutil_count_to_sz(ACLEXPR_OPBINCFG_END + 3)
			+ (sizeof(aclexpr_opbincfg_extra) * 3)
			);
	ae2f_ifezerr(NEW_SCALE) return c_lbl_fail;

	VEC = get_buf_from_scale2(aclspv_wrd_t, h_cmdscale, *NEW_SCALE);
	ae2f_ifezerr(VEC) return c_lbl_fail;

	VEC[ACLEXPR_OPBINCFG_NUMOPRND]	= 3;
	VEC[ACLEXPR_OPBINCFG_OPCODE]	= ACLEXPR_OP3_COND;
	VEC[ACLEXPR_OPBINCFG_RESTYID]	= TY;

	EXTRAS = (void* ae2f_restrict)(&VEC[ACLEXPR_OPBINCFG_END + 3]);
	aclmemzero(EXTRAS, sizeof(aclexpr_opbincfg_extra) * 3);

	/** condition, true, false — in source order. */
	GK.m_cursor	= CH;
	GK.m_res	= CXChildVisit_Continue;
	GK.m_num_wanted	= 3;
	GK.m_num_grepped = 0;

	(void)clang_visitChildren(c_cur, s_aclattr_getkinder_visit, &GK);

	if(GK.m_num_grepped == 3) {
		EXTRAS[0].m_cursor = CH[0];
		EXTRAS[1].m_cursor = CH[1];
		EXTRAS[2].m_cursor = CH[2];
	}

	return c_lbl_success;
}

#endif
