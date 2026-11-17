/** this is not to be necessarily public. */
#ifndef	aclspv_vec_h
#define aclspv_vec_h

#include "./ctx/common.h"
#include "./wrd.h"

#include <.ae2f/pat/vec.h>
#include <stddef.h>

#define	aclspv_vec(T) struct			\
{						\
	h_aclspv_ctx_t m_ctx;			\
	void*	ae2f_restrict m_base;		\
	T*	ae2f_restrict m_vec;		\
	size_t m_sz; aclspv_wrd_t m_nel;	\
}

typedef struct {
	/** should be used */
	void* ae2f_restrict 	m_p;

	/** should be freed */
	void* ae2f_restrict 	m_base;
	h_aclspv_ctx_t		m_ctx;

	aclspv_wrd_t		m_sz, m_sz_real;
} x_aclspv_vec;

#endif
