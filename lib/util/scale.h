/**
 * @file	scale.h
 * @brief	Kind of single linked list where only last one could be resized.
 * */

#ifndef aclutil_scale_h
#define aclutil_scale_h

#include <aclspv.h>
#include <aclmem.h>
#include <ae2f/pat/dbg.h>

typedef struct x_scale x_scale;
typedef x_scale* ae2f_restrict h_scale_t;

struct x_scale {
	aclspv_wrd_t		m_id;
	aclspv_wrd_t		m_nxt;
	aclspv_wrd_t		m_prv;
	aclspv_wrd_t		m_buf;
	aclspv_wrd_t		m_sz;
};

#define	SCALE_HEADER_SIZE	\
	((aclspv_wrd_t)(sizeof(x_scale) + sizeof(aclspv_wrd_t)))

#define	get_scale_header_from_vec(h_alloc)	((aclspv_wrd_t* ae2f_restrict)((h_alloc)->m_p))
#define get_first_scale_from_vec(h_alloc)	((h_scale_t)(get_scale_header_from_vec(h_alloc) + 1))

#define	get_last_scale_from_vec(h_alloc)	\
	((h_scale_t)( (uintptr_t)(*get_scale_header_from_vec(h_alloc)) + (uintptr_t)((h_alloc)->m_p) ))

#define	get_buf_from_scale(h_alloc, c_scale)	\
	(ae2f_expected((h_alloc)->m_p) \
	 ? (void* ae2f_restrict)(((uintptr_t)(h_alloc)->m_p) + ((uintptr_t)(c_scale).m_buf)) \
	 : ((void* ae2f_restrict)ae2f_NIL))

#define	get_buf_from_scale2(buf_T, h_alloc, c_scale)	\
	((buf_T* ae2f_restrict)get_buf_from_scale(h_alloc, c_scale))

#define	get_nxt_from_scale(h_alloc, c_scale)	\
	((x_scale* ae2f_restrict)(((uintptr_t)(h_alloc)->m_p) + ((uintptr_t)(c_scale).m_nxt)))

#define	get_prv_from_scale(h_alloc, c_scale)	\
	((x_scale* ae2f_restrict)(((uintptr_t)(h_alloc)->m_p) + ((uintptr_t)(c_scale).m_prv)))

/** 
 * @fn	 	init_scale
 * @brief	intialise the scale
 * @return	pointer to the first scale of `h_alloc`. `ae2f_NIL` when failed.
 * */
ae2f_inline ae2f_retnew static x_scale* init_scale(
		x_aclspv_vec* ae2f_restrict const	h_alloc,
		const aclspv_wrd_t			c_init_sz
		)
{
	ae2f_ifnezerr_strict(s_aclmem_calloc2(
			h_alloc
			, (aclspv_wrd_t)c_init_sz + SCALE_HEADER_SIZE
			, 1
			, ACLSPV_COMPILE_ALLOC_FAILED
			)) return 0;

	*get_scale_header_from_vec(h_alloc) = sizeof(aclspv_wrd_t);

	get_first_scale_from_vec(h_alloc)->m_buf = SCALE_HEADER_SIZE;
	get_first_scale_from_vec(h_alloc)->m_sz = c_init_sz;
	get_first_scale_from_vec(h_alloc)->m_nxt = 0;
	get_first_scale_from_vec(h_alloc)->m_prv = sizeof(aclspv_wrd_t);
	get_first_scale_from_vec(h_alloc)->m_id = 0;

	assert(get_first_scale_from_vec(h_alloc) == get_last_scale_from_vec(h_alloc));
	return get_first_scale_from_vec(h_alloc);
}

/**
 * @fn		init_scale
 * @brief	get the pointer of `c_idx`th scale from `h_alloc`
 * @return	`ae2f_NIL` on failure.
 * */
ae2f_inline ae2f_ccconst static x_scale* get_scale_from_vec(
		x_aclspv_vec* ae2f_restrict const	h_alloc,
		const aclspv_wrd_t			c_idx
		)
{
	aclspv_wrd_t i = c_idx;
	x_scale* ae2f_restrict ret = get_first_scale_from_vec(h_alloc);
	ae2f_expected_but_else(ret) {
		assert("non-alloc" && 0);
		return ae2f_NIL;
	}

	ae2f_unexpected_but_if(h_alloc->m_sz < SCALE_HEADER_SIZE) {
		assert("misconfigured" && 0);
		return ae2f_NIL;
	}

	while(i--) {
		ae2f_expected_but_else(ret->m_nxt) {
			ae2fpat_assertassume_strict("exceeded" && 0);
			ae2f_unreachable();
			return ae2f_NIL;
		}

		ret = get_nxt_from_scale(h_alloc, *ret);
	}

	ae2fpat_assertassume_strict(ret->m_id == c_idx);
	return ret;
}

/**
 * @fn		grow_last_scale
 * @brief	grow the last scale of `h_alloc` in `c_newsize`
 * @return	0 when succeed.
 * */
ae2f_inline	static aclspv_bool_t grow_last_scale(
		x_aclspv_vec* ae2f_restrict const	h_alloc,
		const aclspv_wrd_t			c_newsize
		)
{

	assert(h_alloc);
	unless(h_alloc->m_p) {
		assert(0);
		return 1;
	}

	if(get_last_scale_from_vec(h_alloc)->m_nxt)	return 1;


	ae2f_ifnezerr_strict(aclmem_cgrow(
			h_alloc
			, c_newsize + get_last_scale_from_vec(h_alloc)->m_buf
			)) return 1;

	get_last_scale_from_vec(h_alloc)->m_sz = c_newsize;

	return (aclspv_bool_t)(!ae2f_expected_strict((h_alloc && h_alloc->m_p)));
}

ae2f_retnew ae2f_inline static x_scale* del_scale_from_vec_last(x_aclspv_vec* ae2f_restrict const h_alloc) {
	h_scale_t	lst = get_last_scale_from_vec(h_alloc);
	h_scale_t	lst_prv;

	*get_scale_header_from_vec(h_alloc)
		= lst->m_prv;

	ae2fpat_assertassume_strict(lst);
	lst_prv	= get_prv_from_scale(h_alloc, *lst);
	ae2fpat_assertassume_strict(lst_prv);
	lst_prv->m_nxt = 0;

	return		lst_prv;
}

/**
 * @fn		mk_scale_from_vec
 * @brief	make new scale from `h_alloc` and returns its pointer
 * @details	pointer of new scale or `ae2f_NIL` when failed.
 * */
ae2f_retnew ae2f_inline static x_scale* mk_scale_from_vec(
		x_aclspv_vec* ae2f_restrict const	h_alloc,
		const aclspv_wrd_t			c_init_size_opt
		)
{
	h_scale_t	
		lst = get_last_scale_from_vec(h_alloc)
		, nxt;
	const aclspv_wrd_t newsz = lst->m_buf + lst->m_sz + c_init_size_opt + (aclspv_wrd_t)sizeof(x_scale);

	assert(lst); 
	assert(h_alloc);
	assert(h_alloc->m_p);
	assert(*get_scale_header_from_vec(h_alloc));
	assert(lst->m_buf);

	ae2f_ifnezerr_strict(aclmem_calloc(h_alloc, newsz))
			return 0;

	assert(h_alloc->m_p);
	ae2f_expected_but_else(h_alloc->m_p) return ae2f_NIL;

	lst = get_last_scale_from_vec(h_alloc);
	lst->m_nxt = lst->m_buf + lst->m_sz;

	*get_scale_header_from_vec(h_alloc)
		= lst->m_nxt;

	nxt = get_nxt_from_scale(h_alloc, *lst);

	assert(nxt != lst);

	nxt->m_buf = lst->m_nxt + (aclspv_wrd_t)sizeof(x_scale);
	nxt->m_sz = c_init_size_opt;
	nxt->m_nxt = 0;
	nxt->m_id = lst->m_id + 1;
	nxt->m_prv = (aclspv_wrd_t)((const char*)lst - (const char*)h_alloc->m_p);

	assert(get_first_scale_from_vec(h_alloc) < get_last_scale_from_vec(h_alloc));
	assert(lst->m_buf);
	assert(lst->m_nxt);
	assert(nxt->m_buf);

	return nxt;
}



#endif
