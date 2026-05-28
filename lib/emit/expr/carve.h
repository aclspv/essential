#ifndef	aclexpr_carve_h
#define	aclexpr_carve_h

#include <util/op.h>
#include "../expr.h"

/**
 * @returns 0 when success
 * */
aclspv_bool_t aclexpr_carve(
		struct aclexpr_visit_data* const ae2f_restrict	rdwr_data,
		aclspv_wrd_t* const ae2f_restrict rdwr_prv_idval,
		util_opbincfg_extra* const ae2f_restrict rdwr_prv_extra
		);

#endif
