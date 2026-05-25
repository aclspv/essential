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
#include <dbg/mallocfree.h>

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
			}

			f_free((rc_vec).m_p, (rc_vec).m_sz);
			(rc_vec).m_p = L_new;
			(rc_vec).m_sz = (L_new$$size);
		} else {
			f_free((rc_vec).m_p, (rc_vec).m_sz);
			(rc_vec).m_p = 0;
			(rc_vec).m_sz = 0ul;
		}
	}
}

#endif
