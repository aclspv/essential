#ifndef	aclexpr_aftervisit_h
#define	aclexpr_aftervisit_h

#include <aclexpr.h>

/** 1 for something wrong.
 *
 * `c_cur` is the cursor being entered. for a binary operator's second
 * operand this is the RHS cursor, which the short-circuit lane needs in
 * order to decide whether the RHS is free of side effects.
 * */
aclspv_bool_t aclexpr_beforevisit(
		CXCursor					c_cur,
		struct aclexpr_visit_data* const ae2f_restrict	rdwr_data,
		aclspv_wrd_t					c_opidx
		);

#endif
