#ifndef	aclexpr_carve_h
#define	aclexpr_carve_h

#include <aclexpr/op.h>
#include <aclexpr.h>

/**
 * carve the expression whose scale is on top of the evaluator's stack.
 *
 * a lane's ACLEXPR_CARVE_NO reaches the context as an error here: a carve
 * that cannot be emitted must not let evaluation continue with a
 * half-carved expression, which is how a store could go missing from an
 * otherwise valid-looking module.
 *
 * @returns 0 when success
 * */
aclspv_bool_t aclexpr_carve(
		struct aclexpr_visit_data* const ae2f_restrict	rdwr_data,
		aclspv_wrd_t* const ae2f_restrict rdwr_prv_idval,
		aclexpr_opbincfg_extra* const ae2f_restrict rdwr_prv_extra
		);

#endif
