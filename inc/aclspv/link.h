#ifndef	aclspv_link_h
#define aclspv_link_h

#include <ae2f/cc/dylink.h>

/**
 * @def ACLSPV_ABI_IMPL	
 * @brief Implementation as ABI
 *
 * @def ACLSPV_ABI_DECL	
 * @brief Declaration as ABI
 * */

#ifndef	ACLSPVLIB
#	define	ACLSPVLIB	0
#endif

#if	__ACLSPV_SHARED
#	if ACLSPVLIB
#		define ACLSPV_ABI_LINK	ae2f_impl
#	else
#		define ACLSPV_ABI_LINK	ae2f_decl
#	endif
#else
#	define	ACLSPV_ABI_LINK
#endif

#endif
