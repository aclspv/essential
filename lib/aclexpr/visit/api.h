#ifndef	aclexpr_visit_api_h
#define	aclexpr_visit_api_h

#include <util/cgoto.h>

#if	aclspv_cgoto
typedef	void* 			aclexpr_visit_lbl_t;
#else
#include <ae2f/c90/StdInt/enum.h>
typedef aclspv_wrd_t		aclexpr_visit_lbl_t;
#endif



#endif
