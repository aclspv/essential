#ifndef	aclconst_specsize_h
#define aclconst_specsize_h

#include <aclspv.h>

typedef struct {
	aclspv_id_t	m_id;

	union {
		aclspv_wrd_t	m_wrd;
		aclspv_wrd2_t	m_wrd2;
	}		m_lit;
} aclconst_specsz;

#endif
