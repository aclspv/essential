#ifndef	aclexpr_visit_api_h
#define	aclexpr_visit_api_h

#include <util/cgoto.h>
#include <aclspv.h>

#if	aclspv_cgoto
typedef	void* 			aclexpr_visit_lbl_t;
#else
typedef aclspv_wrd_t		aclexpr_visit_lbl_t;
#endif



#endif
