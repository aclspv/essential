#ifndef	acltrl_h
#define acltrl_h

#include <aclspv.h>

enum ACLTRL_MASK_ {
	ACLTRL_MASK_RT,
	ACLTRL_MASK_CONST	= 0x1,
	ACLTRL_MASK_LTRL	= 0x3
};

enum ACLTRL_ {
	ACLTRL_RT,
	ACLTRL_CONST,
	ACLTRL_LTRL
};

typedef aclspv_wrd_t	acltrl_mask_t;

typedef union {
	int		m_api_int;

	aclspv_swrd2_t	m_api_intmax;
	aclspv_wrd2_t	m_api_uintmax;

	aclspv_wrd2_t	m_u64;

	double		m_dbl;
	float		m_flt;

	aclspv_wrd_t	m_wrd;
	aclspv_wrd2_t	m_wrd2;
} acltrl_t;

struct acltrl_eval {
	acltrl_t	m_ret;
	aclspv_bool_t	m_found;
};

#endif
