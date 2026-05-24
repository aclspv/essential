#ifndef	aclspv_pass_ctx_auto_h
#define	aclspv_pass_ctx_auto_h

#include <stddef.h>
#include <aclspv.h>
#include <ae2f/Keys.h>
#include <ae2f/Cast.h>
#include <ae2f/pat/macrofunc.h>

#if	!__ae2f_MACRO_GENERATED

void f_free(void* ae2f_restrict, size_t);
ae2f_retnew void*	f_malloc(size_t);
void*	f_memcpy(void*, const void*, size_t);

#endif

#include <string.h>
#include <stdlib.h>
#include <util/log.h>

#if	DBG
ae2f_retnew ae2f_inline_f static void*	_aclspv_malloc(size_t c_sz) {
	void* R = calloc(c_sz, 1);
	dbg_prefix(a);
	fprintf(stderr
			, "_aclspv_malloc\n"
			"\tsize_t c_sz = %lu\n"
			"\treturn: %p\n"
			, (unsigned long)c_sz
			, R
			);
	fflush(stderr);
	return R;
}

ae2f_inline_f static void _aclspv_free(void* c_ptr, const size_t c_sz) {
	dbg_prefix(a);
	fprintf(stderr
			, "_aclspv_free\n"
			"\tvoid* c_ptr = %p\n"
			"\tsize_t c_sz = %lu\n"

			, (void*)c_ptr, (const size_t)c_sz
			);
	fflush(stderr);
	free(c_ptr);
}

ae2f_inline_f static void _aclspv_memcpy(void* ae2f_restrict const wr_dst, const void* ae2f_restrict const rd_src, const size_t c_btsize) {
	dbg_prefix();
	fprintf(stderr
			, "_aclspv_memcpy\n"
			"\twr_dst: %p\n"
			"\trd_src: %p\n"
			"\tc_btsize: %lu\n"
			, wr_dst, rd_src, (unsigned long)c_btsize);

	memcpy(wr_dst, rd_src, c_btsize);
	return;
}

#else
#define _aclspv_malloc(a)	calloc(1ae2f_static_cast(size_t, a), 1)
#define _aclspv_free(a, b)	free(a)
#define _aclspv_memcpy(a, b, c)	memmove(a, b, ae2f_static_cast(size_t, c))
#endif

ae2f_MAC() aclspv_init_vec(x_aclspv_vec	i_vec) {
	(i_vec).m_p = ae2f_NIL;
	(i_vec).m_sz = 0ul;
}

ae2f_MAC((f_free, )) aclspv_stop_vec(x_aclspv_vec	s_vec) {
	f_free((s_vec).m_p, (s_vec).m_sz);
}

ae2f_MAC((f_malloc, f_free, ))
	aclspv_grow_vec(x_aclspv_vec rc_vec, const size_t c_new_sz) {
		if((c_new_sz) > (rc_vec).m_sz) {
			f_free((rc_vec).m_p, (c_new_sz));
			ae2f_ifneznrm(((rc_vec).m_p = f_malloc(c_new_sz))) {
				(rc_vec).m_sz = (c_new_sz);
			} else {
				(rc_vec).m_sz = 0ul;
			}
		}
	}

ae2f_MAC((f_malloc, f_free, f_memcpy, L_new, )) aclspv_grow_vec_with_copy(x_aclspv_vec rc_vec, const size_t c_new_sz) {
	const size_t	L_new$$size = c_new_sz;
	if((L_new$$size) > (rc_vec).m_sz) {
		void* const ae2f_restrict 
			L_new = f_malloc(c_new_sz);

		ae2f_ifneznrm(L_new) {
			if((rc_vec).m_p) {
				if((rc_vec).m_sz && (rc_vec).m_p != L_new)
					f_memcpy(L_new, (rc_vec).m_p, (rc_vec).m_sz);
				f_free((rc_vec).m_p, (rc_vec).m_sz);
			}
			(rc_vec).m_p = L_new;
			(rc_vec).m_sz = (L_new$$size);
		} else {
			if((rc_vec).m_p) f_free((rc_vec).m_p, (rc_vec).m_sz);
			(rc_vec).m_p = 0;
			(rc_vec).m_sz = 0ul;
		}
	}
}

#endif
