#ifndef	aclutil_cxtp_implutil_h
#define aclutil_cxtp_implutil_h

#include "./common.h"
#include <string.h>

#if	1


#define	__get_type_uniques(h_ctx) ((aclty_t* ae2f_restrict)h_ctx->m_type_uniques.m_p)


#include <aclmem.h>

struct __ret {
	aclspv_id_t		m_id;
	aclspv_bool_t		m_isfirst;
};

ae2f_inline_f static struct __ret s_ret_new(
		const h_aclspv_ctx_t h_ctx,
		const aclty_t* ae2f_restrict rd_cxtp,
		aclspv_id_t c_id,
		aclspv_bool_t c_can_use_zero
		) {
#define	_return(c_id, c_is_first)	{ RET.m_id = c_id; RET.m_isfirst = c_is_first; return RET; }

	struct	__ret		RET;

	aclspv_wrd_t IDX;
	for(IDX = h_ctx->m_num_type_uniques; IDX --> !c_can_use_zero;) {
		aclty_t* const ae2f_restrict ndf = &__get_type_uniques(h_ctx)[IDX];

		if(ndf->m_class == ACLTY_RESERVED) {
			unless(c_id) {
				c_id = ndf->m_type_id;
			}

			*ndf = *rd_cxtp;
			ndf->m_type_id = c_id;
			_return(c_id, 1);
		}
	}

	IDX = h_ctx->m_num_type_uniques;

	ae2f_ifnezerr_strict(s_aclmem_cgrow2(
				&h_ctx->m_type_uniques
				, h_ctx->m_num_type_uniques + 1
				, sizeof(aclty_t)
				, ACLSPV_COMPILE_ALLOC_FAILED_CXTP
				)) _return(0, 0);

	(void)++h_ctx->m_num_type_uniques;

	{
		unless(c_id) {
			c_id = h_ctx->m_id++;
		}
		__get_type_uniques(h_ctx)[IDX] = *rd_cxtp;
		__get_type_uniques(h_ctx)[IDX].m_type_id = c_id;
		_return(c_id, 1);
	}
#undef	_return
}

#endif

#endif
