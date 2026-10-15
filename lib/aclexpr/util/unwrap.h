#ifndef	aclexpr_util_unwrap_h
#define	aclexpr_util_unwrap_h

#include <aclspv.h>
#include <stdint.h>

#include <aclattr/getkinder.h>

/** the transparent wrappers libclang places between an operator and its
 * operand: user parentheses (ParenExpr) and the UnexposedExpr it surfaces an
 * implicit cast (LValueToRValue / array decay) as.
 *
 * clang_getCursorReferenced forwards through an UnexposedExpr only while its
 * child is a bare DeclRefExpr; the moment a ParenExpr sits under it the
 * referent is lost. every carve-side peek must descend through BOTH before
 * classifying an operand. */

/** descend through ParenExpr / UnexposedExpr to the underlying expression.
 * a wrapper with no child stops the descent; bounded so a malformed cursor
 * graph cannot loop forever. */
ae2f_inline_f static CXCursor aclexpr_expr_unwrap(CXCursor c_cur) {
	aclspv_wrd_t bound = 32;

	while(bound--
			&& (c_cur.kind == CXCursor_ParenExpr
				|| c_cur.kind == CXCursor_UnexposedExpr)) {
		CXCursor c_next = clang_getNullCursor();
		struct aclattr_getkinder	K;

		K.m_cursor		= &c_next;
		K.m_res			= CXChildVisit_Break;
		K.m_num_wanted		= 1;
		K.m_num_grepped		= 0;

		clang_visitChildren(c_cur, s_aclattr_getkinder_visit, &K);

		if(clang_Cursor_isNull(c_next))
			break;

		c_cur = c_next;
	}

	return c_cur;
}

/** the l-value form of an expression, after transparent unwrapping: the one
 * classification shared by the assign lane (which picks a write carve from it)
 * and every write carve (which re-derives its operand from it). */
enum ACLEXPR_LVALUEKIND_ {
	ACLEXPR_LVALUEKIND_NONE = 0,
	ACLEXPR_LVALUEKIND_DECLREF,
	ACLEXPR_LVALUEKIND_DEREF,
	ACLEXPR_LVALUEKIND_MEMBER,
	ACLEXPR_LVALUEKIND_SUBSCRIPT
};

ae2f_inline_f static enum ACLEXPR_LVALUEKIND_ aclexpr_lvalue_kind(CXCursor c_cur) {
	c_cur = aclexpr_expr_unwrap(c_cur);

	switch((int)c_cur.kind) {
		case CXCursor_UnaryOperator:
			return clang_getCursorUnaryOperatorKind(c_cur)
					== CXUnaryOperator_Deref
				? ACLEXPR_LVALUEKIND_DEREF
				: ACLEXPR_LVALUEKIND_NONE;

		case CXCursor_MemberRefExpr:
			return ACLEXPR_LVALUEKIND_MEMBER;

		case CXCursor_ArraySubscriptExpr:
			return ACLEXPR_LVALUEKIND_SUBSCRIPT;

		case CXCursor_DeclRefExpr:
			return ACLEXPR_LVALUEKIND_DECLREF;

		default:
			return ACLEXPR_LVALUEKIND_NONE;
	}
}

#endif
