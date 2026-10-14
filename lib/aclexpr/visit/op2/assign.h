#ifndef	aclexpr_visit_op2_assign_h
#define	aclexpr_visit_op2_assign_h

#include "../api.h"
#include "../../op.h"

#include <aclexpr/util/unwrap.h>

#include <util/constant.h>
#include <util/scale.h>
#include <util/wrdemit.h>

#include <aclty.h>
#include <aclmem.h>

/** capture the first child of the binary operator (clang publishes the
 * assignment operands in order: LHS first). the assign stage uses it to
 * detect a member lvalue (`s.b`) before the generic child cascade runs. */
typedef struct {
	CXCursor	m_c_first;
	aclspv_bool_t	m_has;
} s_assign_peek_t;

static enum CXChildVisitResult s_assign_peek(
		CXCursor			c_cur,
		ae2f_unused CXCursor		c_par,
		CXClientData			rdwr_data
		) {
	s_assign_peek_t* ae2f_restrict h = rdwr_data;

	h->m_c_first = c_cur;
	h->m_has = 1;
	return CXChildVisit_Break;
}

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
	s_assign_peek_t			PEEK;

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

	/** lvalue target: classify the (transparently unwrapped) first child so
	 * a parenthesized / implicit-cast-wrapped lvalue routes the same as the
	 * bare one — `*(p) = x`, `(*p) = x`, `(s).m = x`, `(a[i]) = x` all reach
	 * the matching write carve. the NO_NEED seed below still applies: every
	 * write carve honours it by skipping the read cascade. */
	PEEK.m_c_first = clang_getNullCursor();
	PEEK.m_has = 0;
	clang_visitChildren(c_cur, s_assign_peek, &PEEK);
	if(PEEK.m_has) {
		switch((int)aclexpr_lvalue_kind(PEEK.m_c_first)) {
			case ACLEXPR_LVALUEKIND_MEMBER:
				VEC[ACLEXPR_OPBINCFG_OPCODE] = ACLEXPR_OP2_MEMBERWRITE;
				break;

			case ACLEXPR_LVALUEKIND_DEREF:
				VEC[ACLEXPR_OPBINCFG_OPCODE] = ACLEXPR_OP2_DEREFWRITE;
				break;

			case ACLEXPR_LVALUEKIND_SUBSCRIPT:
				VEC[ACLEXPR_OPBINCFG_OPCODE] = ACLEXPR_OP2_SUBSCRIPTWRITE;
				break;

			default:
				break;
		}
	}

	aclmemzero(&VEC[ACLEXPR_OPBINCFG_END + 2], sizeof(aclexpr_opbincfg_extra) << 1);
	((aclexpr_opbincfg_extra* ae2f_restrict)(void*)&VEC[ACLEXPR_OPBINCFG_END + 2])->m_state0 = (aclspv_wrd_t)ACLEXPR_OPCFGX_STATE0_VAL_NO_NEED;

	return c_lbl_success;
}

#endif
