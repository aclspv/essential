/**
 * @file	initlist.h
 * @brief	aggregate value production: C initializer lists `{ a, b, c, ... }`
 *
 *	claims CXCursor_InitListExpr, counts the element expressions to shape an
 *	N-operand scale, and records the aggregate result type (vector / array /
 *	struct ... whatever aclty can model from the cursor type) into RESTYID.
 *	the carve lane resolves the class and emits either OpConstantComposite
 *	(all elements foldable) or an OpCompositeConstruct opX node (any runtime
 *	element); struct/union targets are a clean error (next lane).
 * */

#ifndef	aclexpr_visit_opn_initlist_h
#define	aclexpr_visit_opn_initlist_h

#include "../../op.h"
#include "../api.h"

#include <aclcur/opX.h>

#include <util/scale.h>
#include <util/wrdemit.h>

#include <aclty.h>
#include <aclmem.h>

static enum CXChildVisitResult s_opn_initlist_cnt(
		ae2f_unused CXCursor			c_cur,
		ae2f_unused CXCursor			c_par,
		CXClientData				rdwr_data
		) {
	(void)++*(aclspv_wrd_t* ae2f_restrict)rdwr_data;
	return CXChildVisit_Continue;
}

ae2f_inline_f static aclexpr_visit_lbl_t aclexpr_visit_opn_initlist(
		const CXCursor				c_cur,
		const h_aclspv_ctx_t			h_ctx, 
		x_aclspv_vec* ae2f_restrict const	h_cmdscale,
		aclexpr_visit_lbl_t			c_lbl_success,
		aclexpr_visit_lbl_t			c_lbl_fail,
		aclexpr_visit_lbl_t			c_lbl_pass
		)
{
	x_scale* ae2f_restrict		NEW_SCALE;
	aclspv_wrd_t* ae2f_restrict	VEC;
	aclspv_wrd_t			N;
	aclspv_id_t			TY_RES;
	aclspv_wrd_t			M;

	ae2fpat_assertunless(h_ctx) {
		return c_lbl_fail;
	}

	unless(CXCursor_InitListExpr == c_cur.kind)
		return c_lbl_pass;

	N = 0;
	clang_visitChildren(c_cur, s_opn_initlist_cnt, &N);

	unless(N)
		return c_lbl_fail;

	ae2f_ifezerr(TY_RES = aclty_mk_by_cursor_dummy(
				c_cur
				, clang_getCanonicalType(clang_getCursorType(c_cur))
				, h_ctx
				)) {
		err_puts("[visit::opn::initlist] failed to make the result type");
		h_ctx->m_err = ACLSPV_COMPILE_CXTP_ZERO;
		return c_lbl_fail;
	}

		/** a C90 union takes a single initializer for its first member, so the
	 * carve lane pads the rest with nulls. reserve member-count operand
	 * slots and extras for union targets so that padding stays in bounds. */
	M = N;
	{
		const aclty_t* const ae2f_restrict P = aclty_find(TY_RES, h_ctx);

		if(P && P->m_class == ACLTY_STRUCT
				&& (P->m_field0 & ACLTY_STRUCT_FIELD0_UNION)
				&& P->m_info.m_struct.m_num_mbr > N
				&& P->m_info.m_struct.m_num_mbr <= ACLCUR_OPX_OPRND_CAP)
			M = P->m_info.m_struct.m_num_mbr;
	}

	NEW_SCALE = get_last_scale_from_vec(h_cmdscale);
	NEW_SCALE = mk_scale_from_vec(
			h_cmdscale
			, 
			(aclspv_wrd_t)(
				aclutil_count_to_sz(ACLEXPR_OPBINCFG_END + M)
				+ (sizeof(aclexpr_opbincfg_extra) * M)
				)
			);
	ae2f_expected_but_else(NEW_SCALE)
		return c_lbl_fail;

	VEC = get_buf_from_scale2(aclspv_wrd_t, h_cmdscale, *NEW_SCALE);

	ae2f_expected_but_else(VEC)
		return c_lbl_fail;

	VEC[ACLEXPR_OPBINCFG_NUMOPRND]	= N;
	VEC[ACLEXPR_OPBINCFG_OPCODE]	= ACLEXPR_OPN_INITLIST;
	VEC[ACLEXPR_OPBINCFG_RESTYID]	= TY_RES;

	aclmemzero(
			&VEC[ACLEXPR_OPBINCFG_END + M]
			, (aclspv_wrd_t)(sizeof(aclexpr_opbincfg_extra) * M)
			);

	return c_lbl_success;
}

#endif
