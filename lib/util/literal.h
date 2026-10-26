#ifndef aclutil_literal_h
#define aclutil_literal_h

#include "./iddef.h"

enum ACLUTIL_LITERAL_MASK_ {
	ACLUTIL_LITERAL_MASK_RT,
	ACLUTIL_LITERAL_MASK_CONSTANT	= 0x1,
	ACLUTIL_LITERAL_MASK_LITERAL	= 0x3
};

enum ACLUTIL_LITERAL_ {
	ACLUTIL_LITERAL_RT,
	ACLUTIL_LITERAL_CONSTANT,
	ACLUTIL_LITERAL_LITERAL
};

#include <ae2f/c90/stdint.h>

typedef aclspv_wrd_t	aclutil_ltrlmask_t;
typedef union {
	int		m_api_int;

	aclspv_swrd2_t	m_api_intmax;
	aclspv_wrd2_t	m_api_uintmax;

	aclspv_wrd2_t	m_u64;

	double		m_dbl;
	float		m_flt;

	aclspv_wrd_t	m_wrd;
	aclspv_wrd2_t	m_wrd2;
} aclutil_literal;

struct aclutil_ltrl_eval_t {
	aclutil_literal	m_ret;
	aclspv_bool_t	m_found;
};



#endif
