#ifndef aclutil_literal_h
#define aclutil_literal_h

#include "./iddef.h"
#include "./log.h"

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

typedef aclspv_wrd_t	aclutil_ltrlmask_t;
typedef union {
	int		m_api_int;
	intmax_t	m_api_intmax;
	uintmax_t	m_api_uintmax;

	u64_least	m_u64;
	double		m_dbl;
	float		m_flt;
} aclutil_literal;

struct aclutil_ltrl_eval_t {
	aclutil_literal	m_ret;
	aclspv_bool_t	m_found;
};



#endif
