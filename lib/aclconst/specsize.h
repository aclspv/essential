#ifndef	aclconst_specsize_h
#define aclconst_specsize_h

#include <aclspv.h>

typedef struct {
	aclspv_id_t	m_id;

	union {
		aclspv_wrd_t	m_wrd;
		aclspv_wrd2_t	m_wrd2;
		aclspv_wrd2_t	m_max;
	}		m_lit;
} aclconst_specsz;

aclspv_bool_t aclconst_is_specsz_same(aclconst_specsz c_x, aclconst_specsz c_y, h_aclspv_ctx_t h_ctx);

#endif
