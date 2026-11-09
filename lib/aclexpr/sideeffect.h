#ifndef	aclexpr_sideeffect_h
#define	aclexpr_sideeffect_h

#include <aclspv.h>
#include <ae2f/c90/stdint.h>

/** true when this cursor kind may be evaluated for its value without an
 * observable side effect. unknown kinds are impure (conservative). */
static aclspv_bool_t	s_cursor_kind_pure(CXCursor c_cur) {
	switch((uintmax_t)clang_getCursorKind(c_cur)) {
		default:
			return 0;

		case CXCursor_UnexposedExpr:
		case CXCursor_IntegerLiteral:
		case CXCursor_FloatingLiteral:
		case CXCursor_CharacterLiteral:
		case CXCursor_StringLiteral:
		case CXCursor_DeclRefExpr:
		case CXCursor_ParenExpr:
		case CXCursor_CStyleCastExpr:
		case CXCursor_ConditionalOperator:
		case CXCursor_ArraySubscriptExpr:
		case CXCursor_MemberRefExpr:
		case CXCursor_InitListExpr:
		case CXCursor_CompoundLiteralExpr:
		case CXCursor_UnaryExpr:
			return 1;

		case CXCursor_UnaryOperator:
			switch((uintmax_t)clang_getCursorUnaryOperatorKind(c_cur)) {
				case CXUnaryOperator_PostInc:
				case CXUnaryOperator_PostDec:
				case CXUnaryOperator_PreInc:
				case CXUnaryOperator_PreDec:
					return 0;
				default:
					return 1;
			}

		case CXCursor_BinaryOperator:
			switch((uintmax_t)clang_getCursorBinaryOperatorKind(c_cur)) {
				case CXBinaryOperator_Assign:
				case CXBinaryOperator_AddAssign:
				case CXBinaryOperator_SubAssign:
				case CXBinaryOperator_MulAssign:
				case CXBinaryOperator_DivAssign:
				case CXBinaryOperator_RemAssign:
				case CXBinaryOperator_ShlAssign:
				case CXBinaryOperator_ShrAssign:
				case CXBinaryOperator_AndAssign:
				case CXBinaryOperator_OrAssign:
				case CXBinaryOperator_XorAssign:
					return 0;
				default:
					return 1;
			}
	}
}

struct s_cursor_pure_ctx {
	aclspv_bool_t	m_impure;
};

static enum CXChildVisitResult s_cursor_pure_visit(
		CXCursor			c_cur,
		ae2f_unused CXCursor		c_par,
		CXClientData			rdwr_data
		) {
	struct s_cursor_pure_ctx* ae2f_restrict p = rdwr_data;

	if(!s_cursor_kind_pure(c_cur)) {
		p->m_impure = 1;
		return CXChildVisit_Break;
	}

	return CXChildVisit_Recurse;
}

/** true when the expression subtree may have an observable side effect. */
static aclspv_bool_t	s_cursor_impure(CXCursor c_cur) {
	struct s_cursor_pure_ctx	p = {0, };

	if(!s_cursor_kind_pure(c_cur))
		return 1;

	(void)clang_visitChildren(c_cur, s_cursor_pure_visit, &p);
	return p.m_impure;
}

#endif
