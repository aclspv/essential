#include <util/cgoto.h>

#if	aclspv_cgoto
typedef	void* 			aclexpr_visit_lbl_t;
#else
#include <ae2f/c90/StdInt/enum.h>
typedef uint_leastenum_t	aclexpr_visit_lbl_t;
#endif
