/** 
 * @file fnctx.h 
 * */

#ifndef	aclutil_fnctx_h
#define	aclutil_fnctx_h

#include <ae2f/pat/vec.auto.h>
#include <ae2f/c90/StdInt.h>
#include <ae2f/cc.h>

#include "./wrd.h"

#include <aclspv.h>

#define	__ENUM()	\
	__X(FNRET),	\
	__X(LBL_DO_SOMETHING),	\
	__X(BREAK_USED)

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
	h_aclspv_ctx_t		m_ctx;
	uintmax_t		m_var_stage;	/** scope level */

	/** @brief see `enum ACLSPV_FNCTX_STATEPAD0_` */
	uint_leastenum_t	m_state0;

	aclspv_id_t		m_retid;		/** return type id */
	aclspv_wrdcount_t	m_lbl_snapshot;		/** last label destination */
	aclspv_wrdcount_t	m_name_snapshot;	/** last name destination */
	aclspv_wrdcount_t	m_break_loop;		/** break destination */
	aclspv_wrdcount_t	m_continue_loop;	/** continue destination */
} aclutil_fnctx;

#endif
