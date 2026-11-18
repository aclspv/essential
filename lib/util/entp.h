/** @file entp.h */

#ifndef __lib_build_entp
#define __lib_build_entp

#include <aclspv.h>

typedef struct {
	aclspv_id_t	m_id;
	aclspv_wrd_t	m_idx_push_constant;
	aclspv_wrd_t	m_descriptor_set;
	CXCursor	m_fn;

	/** function-level purity, captured post-carve by
	 * aclemit_fndecl_bodycfg (the cursor arena is reset at FunctionEnd).
	 * see aclcur_fn_is_const(). */
	aclspv_bool_t	m_is_const;

	/** see aclcur_fn_is_pure(). */
	aclspv_bool_t	m_is_pure;
} aclutil_entp_t;

#endif
