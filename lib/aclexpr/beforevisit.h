#ifndef	aclexpr_aftervisit_h
#define	aclexpr_aftervisit_h

#include <aclexpr.h>

/** 1 for something wrong. */
aclspv_bool_t aclexpr_beforevisit(
		struct aclexpr_visit_data* const ae2f_restrict	rdwr_data,
		aclspv_wrd_t					c_opidx
		);

#endif
