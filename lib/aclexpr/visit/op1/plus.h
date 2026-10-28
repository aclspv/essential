#ifndef	aclexpr_visit_op1_plus_h
#define	aclexpr_visit_op1_plus_h

#include <aclspv.h>
#include <aclexpr/visit/api.h>

static aclexpr_visit_lbl_t aclexpr_visit_op1_plus(
		const CXCursor				c_cur,
		aclexpr_visit_lbl_t			c_lbl_success,
		aclexpr_visit_lbl_t			c_lbl_pass
		) {
	if(c_cur.kind != CXCursor_UnaryOperator)
		return c_lbl_pass;

	if(clang_getCursorUnaryOperatorKind(c_cur) != CXUnaryOperator_Plus)
		return c_lbl_pass;

	return c_lbl_success;
}

#endif
