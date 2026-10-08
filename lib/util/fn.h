/** @file fn.h */

#ifndef lib_build_fn_h
#define lib_build_fn_h

#include "./wrd.h"

typedef struct {
	/* @brief Result ID for function */
	aclspv_id_t	m_id;

	/* @brief OpTypeFunction ID */
	aclspv_id_t	m_type_id;

	/* @brief LLVM function reference */
	CXCursor	m_fn;
} aclutil_fn_t;

#endif
