#ifndef	aclexpr_visit_op1_memberread_h
#define	aclexpr_visit_op1_memberread_h

#include "../../op.h"
#include "../api.h"

#include <util/scale.h>
#include <util/wrdemit.h>

#include <aclty.h>
#include <aclty/by_cursor.h>

#include <aclmem.h>

/** count of FieldDecl children of the target record up to (and including)
 * the referenced field: the member index inside the aclty member chain. */
typedef struct {
	CXCursor	m_c_target;
	aclspv_wrd_t	m_idx;
	aclspv_bool_t	m_found;
} s_memberread_cnt_t;

static enum CXChildVisitResult s_memberread_cnt(
		CXCursor			c_cur,
		ae2f_unused CXCursor		c_par,
		CXClientData			rdwr_data
		) {
	s_memberread_cnt_t* ae2f_restrict h = rdwr_data;

	if(c_cur.kind != CXCursor_FieldDecl)
		return CXChildVisit_Continue;

	if(clang_equalCursors(c_cur, h->m_c_target)) {
		h->m_found = 1;
		return CXChildVisit_Break;
	}

	(void)++h->m_idx;
	return CXChildVisit_Continue;
}

ae2f_inline_f static aclexpr_visit_lbl_t aclexpr_visit_op1_memberread(
		const CXCursor				c_cur,
		const h_aclspv_ctx_t			h_ctx, 
		x_aclspv_vec* ae2f_restrict const	h_cmdscale,
		aclexpr_opbincfg_extra* ae2f_restrict	rdwr_extra,
		aclexpr_visit_lbl_t			c_lbl_success,
		aclexpr_visit_lbl_t			c_lbl_fail,
		aclexpr_visit_lbl_t			c_lbl_pass,
		aclexpr_visit_lbl_t			c_lbl_collect
		)
{
	x_scale* ae2f_restrict		NEW_SCALE;
	aclspv_wrd_t* ae2f_restrict	VEC;
	aclexpr_opbincfg_extra* ae2f_restrict SCALE_EXTRA;
	CXCursor			FD;
	CXType				TY_C;
	aclspv_id_t			TY_M;
	aclspv_wrd_t			IDX;
	s_memberread_cnt_t		CNT;

	ae2fpat_assertunless(h_ctx) {
		return c_lbl_fail;
	}

	unless(CXCursor_MemberRefExpr == c_cur.kind)
		return c_lbl_pass;

	/** assignment-target lvalue: the assign stage pre-seeded
	 * STATE0_VAL_NO_NEED into this operand's extra slot (the same slot
	 * OP0_VAL reads for a plain variable lvalue). no read cascade, no
	 * scale: the member-write carve re-derives the base register and the
	 * member index from this MemberRefExpr cursor (extra[0]). */
	if(rdwr_extra->m_state0
			& (aclspv_wrd_t)ACLEXPR_OPCFGX_STATE0_VAL_NO_NEED) {
		dbg_puts("[visit1::memberread] lvalue target: read skipped");
		rdwr_extra->m_mask_literal = 0;
		return c_lbl_collect;
	}

	FD = clang_getCursorReferenced(c_cur);
	unless(clang_getCursorKind(FD) == CXCursor_FieldDecl)
		return c_lbl_fail;

	TY_C = clang_getCursorType(FD);

	ae2f_ifezerr(TY_M = aclty_mk_by_cursor_dummy(FD, TY_C, h_ctx)) {
		err_puts("[visit::op1::memberread] failed to make the member type");
		h_ctx->m_err = ACLSPV_COMPILE_CXTP_ZERO;
		return c_lbl_fail;
	}

	/** member ordinal: FieldDecl children of the record, in order,
	 * matching the aclty member chain built in by_cursor. */
	CNT.m_c_target = FD;
	CNT.m_idx = 0;
	CNT.m_found = 0;
	clang_visitChildren(
			clang_getCursorSemanticParent(FD)
			, s_memberread_cnt
			, &CNT
			);

	unless(CNT.m_found) {
		err_puts("[visit::op1::memberread] field not found in its record");
		return c_lbl_fail;
	}

	IDX = CNT.m_idx;

	NEW_SCALE = get_last_scale_from_vec(h_cmdscale);
	NEW_SCALE = mk_scale_from_vec(
			h_cmdscale
			, 
			aclutil_count_to_sz(ACLEXPR_OPBINCFG_END + 1)
			+ (sizeof(aclexpr_opbincfg_extra) * 2)
			);
	ae2f_expected_but_else(NEW_SCALE)
		return c_lbl_fail;

	VEC = get_buf_from_scale2(aclspv_wrd_t, h_cmdscale, *NEW_SCALE);

	ae2f_expected_but_else(VEC)
		return c_lbl_fail;

	VEC[ACLEXPR_OPBINCFG_NUMOPRND]	= 1;
	VEC[ACLEXPR_OPBINCFG_OPCODE]	= ACLEXPR_OP1_MEMBERREAD;
	VEC[ACLEXPR_OPBINCFG_RESTYID]	= TY_M;

	/** operand slot 0: filled by OP0_ when the base child completes.
	 * extra[1] carries the member index to the carve lane; it is never
	 * touched by OP0_ (only extra[0] is). */
	VEC[ACLEXPR_OPBINCFG_END]	= 0;

	SCALE_EXTRA = (void* ae2f_restrict)(&VEC[ACLEXPR_OPBINCFG_END + 1]);
	aclmemzero(SCALE_EXTRA, sizeof(aclexpr_opbincfg_extra) * 2);
	SCALE_EXTRA[1].m_literal.m_u64 = IDX;
	/** the member reference itself, so the carve can classify its base
	 * (arrow `p->m` / deref `(*p).m` -> a ghost-rooted view) without
	 * relying on extra[0].m_cursor, which the cascade rewrites. */
	SCALE_EXTRA[1].m_cursor = c_cur;

	return c_lbl_success;
}

#endif
