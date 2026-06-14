#ifndef	aclutil_fnctx_defs_h
#define	aclutil_fnctx_defs_h

#include <ae2f/pat/vec.auto.h>
#include <ae2f/cc.h>
#include <util/wrd.h>

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

enum ACLUTIL_FNCTX_SNAPS_ {
	ACLUTIL_FNCTX_SNAPS_LBL,
	ACLUTIL_FNCTX_SNAPS_END
};

typedef struct {
	h_aclspv_ctx_t		m_ctx;
	aclspv_wrd_t		m_var_stage;	/** scope level */
	aclspv_id_t		m_branch;	/** current branch id */

	/** @brief see `enum ACLSPV_FNCTX_STATEPAD0_` */
	uint_leastenum_t	m_state0;

	aclspv_id_t		m_retid;		/** return type id */

	struct {
		aclspv_wrdcount_t	m_idx_fnimpl;
		aclspv_wrdcount_t	m_idx_fndef;
		aclspv_wrdcount_t	m_idx_name;
	} m_snaps[ACLUTIL_FNCTX_SNAPS_END];

	aclspv_wrdcount_t	m_break_loop;		/** break destination */
	aclspv_wrdcount_t	m_continue_loop;	/** continue destination */
} aclutil_fnctx;

#endif
