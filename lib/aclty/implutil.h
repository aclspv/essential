#ifndef	aclutil_cxtp_implutil_h
#define aclutil_cxtp_implutil_h

#include "./common.h"
#include <string.h>

#if	1
/** 1 when same */
ae2f_inline_f ae2f_ccpure static aclspv_bool_t s_ndeq(
		const aclutil_cxtp* const ae2f_restrict A,
		const aclutil_cxtp* const ae2f_restrict B
		)
{
	if(A->m_class != B->m_class) return 0;
	switch(A->m_class) {
		case ACLUTIL_CXTYPE_FUNC:
		case ACLUTIL_CXTYPE_PTR:
		case ACLUTIL_CXTYPE_VECTOR:
		case ACLUTIL_CXTYPE_ARRAY:
		case ACLUTIL_CXTYPE_FUNC_NOPRM:
		case ACLUTIL_CXTYPE_STRUCT:
		case ACLUTIL_CXTYPE_DCR:
		case ACLUTIL_CXTYPE_STRUCT_MBR:
		case ACLUTIL_CXTYPE_FUNC_ARG:
		default:
			return !memcmp(&A->m_info, &B->m_info, sizeof(A->m_info));

		case ACLUTIL_CXTYPE_UNIT:
			break;

		case ACLUTIL_CXTYPE_RESERVED:
			return 1;
	}

	return 0;
}

ae2f_inline_f ae2f_ccpure static aclspv_wrdcount_t s_find_node(
		const aclutil_cxtp* const ae2f_restrict rd_tar,
		const h_aclspv_ctx_t			h_ctx
		) {
	aclspv_wrdcount_t IDX = (h_ctx)->m_num_type_uniques;
	ae2f_ifezerr(h_ctx->m_type_uniques.m_p)
		return 0xFFFFFFFF;

	ae2f_ifnezerr((h_ctx)->m_num_type_uniques > (aclspv_wrd_t)(h_ctx->m_type_uniques.m_sz / sizeof(aclutil_cxtp)))
		return 0xFFFFFFFF;

	while(IDX-- 
			&& !s_ndeq(
				rd_tar
				, ((const aclutil_cxtp* ae2f_restrict)h_ctx->m_type_uniques.m_p) + IDX))
	{}

	return IDX;
}

#define	__get_type_uniques(h_ctx) ((aclutil_cxtp* ae2f_restrict)h_ctx->m_type_uniques.m_p)


#include <aclmem.h>

struct __ret {
	aclspv_id_t		m_id;
	aclspv_bool_t		m_isfirst;
};

ae2f_inline_f static struct __ret s_ret_new(const h_aclspv_ctx_t h_ctx, const aclutil_cxtp* ae2f_restrict rd_cxtp, aclspv_id_t c_id, aclspv_bool_t c_can_use_zero) {
#define	_return(c_id, c_is_first)	{ RET.m_id = c_id; RET.m_isfirst = c_is_first; return RET; }

	aclspv_wrdcount_t IDX = h_ctx->m_num_type_uniques;
	struct	__ret		RET;

	{
		aclutil_cxtp ndrsv, * ae2f_restrict ndf;
		aclspv_wrd_t	f;
		ndrsv.m_class = ACLUTIL_CXTYPE_RESERVED;
		f = s_find_node(&ndrsv, h_ctx);

		if((f) != 0xFFFFFFFF && (c_can_use_zero ? 1 : !!f)) {
			ndf = &__get_type_uniques(h_ctx)[f];

			unless(c_id) {
				c_id = ndf->m_type_id;
			}

			*ndf = *rd_cxtp;
			ndf->m_type_id = c_id;
			_return(c_id, 1);
		}
	}

	ae2f_ifnezerr_strict(s_aclmem_cgrow2(
				&h_ctx->m_type_uniques
				, h_ctx->m_num_type_uniques + 1
				, sizeof(aclutil_cxtp)
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

ae2f_inline_f static struct __ret s_ret(const h_aclspv_ctx_t h_ctx, const aclutil_cxtp* ae2f_restrict rd_cxtp)
{
	aclspv_wrdcount_t	IDX = s_find_node(rd_cxtp, h_ctx);

	ae2f_ifezerr_strict(IDX == 0xFFFFFFFF) {
		struct	__ret		RET;
		RET.m_id	= __get_type_uniques(h_ctx)[IDX].m_type_id;
		RET.m_isfirst	= 0;
		return RET;
	}

	return s_ret_new(h_ctx, rd_cxtp, 0, 1);
}

#endif

#endif
