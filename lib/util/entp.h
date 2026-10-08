/** @file entp.h */

#ifndef __lib_build_entp
#define __lib_build_entp

#include <util/wrd.h>

typedef struct {
	aclspv_id_t	m_id;
	aclspv_wrd_t	m_idx_push_constant;
	aclspv_wrd_t	m_descriptor_set;
	CXCursor	m_fn;
} aclutil_entp_t;

#endif
