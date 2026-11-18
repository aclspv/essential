/** @file fn.h */

#ifndef lib_build_fn_h
#define lib_build_fn_h

#include <aclspv.h>

typedef struct {
	/* @brief Result ID for function */
	aclspv_id_t	m_id;

	/* @brief OpTypeFunction ID */
	aclspv_id_t	m_type_id;

	/* @brief LLVM function reference */
	CXCursor	m_fn;

	/* @brief function-level purity, captured post-carve by
	 * aclemit_fndecl_bodycfg. see aclcur_fn_is_const(). */
	aclspv_bool_t	m_is_const;

	/* @brief see aclcur_fn_is_pure(). */
	aclspv_bool_t	m_is_pure;
} aclutil_fn_t;

#endif
