#ifndef	aclexpr_carve_h
#define	aclexpr_carve_h

#include <util/op.h>

aclspv_bool_t aclexpr_carve(
		uintptr_t* const ae2f_restrict	rdwr_data,
		aclspv_wrd_t* const ae2f_restrict rdwr_prv_idval,
		util_opbincfg_extra* const ae2f_restrict rdwr_prv_extra
		);

#endif
