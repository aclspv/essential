#ifndef	aclexpr_visit_opn_sin_h
#define	aclexpr_visit_opn_sin_h

#include <aclexpr.h>
#include <aclexpr/visit/api.h>
#include <aclspv.h>

/** handles `__builtin_sinf` / `sinf` (scalar) and
 * `__builtin_elementwise_sin` (scalar or vector) as a one-operand Sin
 * expression.
 * */
aclexpr_visit_lbl_t aclexpr_visit_op1_sin(
		CXCursor				c_cur,
		struct aclexpr_visit_data* ae2f_restrict	rdwr_data,
		aclexpr_visit_lbl_t			c_lbl_success,
		aclexpr_visit_lbl_t			c_lbl_fail,
		aclexpr_visit_lbl_t			c_lbl_pass
		);

#endif
