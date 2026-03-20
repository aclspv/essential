/** 
 * @file fnctx.h 
 * */

#ifndef	aclutil_fnctx_h
#define	aclutil_fnctx_h

#include <ae2f/pat/vec.auto.h>
#include <ae2f/c90/StdInt.h>
#include <ae2f/cc.h>

#include "./ctx.h"

#define	__ENUM()	\
	__X(FNRET)

#define	__X(A)	ACLUTIL_FNCTX_STATEPAD0_##A

enum ACLUTIL_FNCTX_STATEPAD0_ {
	__ENUM()
};

#undef	__X
#define	__X(A)	ACLUTIL_FNCTX_STATE0_##A = 1 << ACLUTIL_FNCTX_STATEPAD0_##A

enum ACLUTIL_FNCTX_STATE0_ {
	__ENUM()
};

#undef	__X
#undef	__ENUM

typedef struct {
	h_util_ctx_t		m_ctx;

	/** @brief see `enum ACLSPV_FNCTX_STATEPAD0_` */
	uint_leastenum_t	m_state0;
} aclutil_fnctx;


#endif
