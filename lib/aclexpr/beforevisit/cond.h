#ifndef	aclexpr_beforevisit_cond_h
#define	aclexpr_beforevisit_cond_h

#include <aclexpr.h>

/** `c ? t : f` — impure-arm branch/phi lane.
 *
 * called for each operand cursor of a conditional operator; `c_opidx`
 * names the operand being entered:
 *
 *   0 = condition, 1 = true arm, 2 = false arm.
 *
 * at idx 1 it decides whether the arms are pure. `OpSelect` evaluates
 * both arms, which is wrong exactly when an arm has an observable side
 * effect, so either impure arm turns the expression into a branch/phi
 * lane. at idx 2 it closes the true-arm block and opens the false one.
 *
 * 1 for something wrong.
 * */
aclspv_bool_t aclexpr_beforevisit3_cond(
		CXCursor					c_cur,
		struct aclexpr_visit_data* const ae2f_restrict	rdwr_data,
		aclspv_wrd_t					c_opidx
		);

#endif
