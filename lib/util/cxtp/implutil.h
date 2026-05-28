#ifndef	aclutil_cxtp_implutil_h
#define aclutil_cxtp_implutil_h

#include "./common.h"
#include <.aclspv/vec.h>
#include <string.h>

ae2f_inline_f static aclspv_bool_t s_ndeq(
		const aclutil_cxtp* const ae2f_restrict A,
		const aclutil_cxtp* const ae2f_restrict B
		)
{
	if(A->m_class != B->m_class) return 0;
	switch(A->m_class) {
		case ACLUTIL_CXTYPE_FUNC:
			return clang_equalTypes(
					A->m_info.m_fn.m_type
					, B->m_info.m_fn.m_type
					) && clang_equalCursors(
						A->m_info.m_fn.m_cursor
						, B->m_info.m_fn.m_cursor);

		case ACLUTIL_CXTYPE_PTR:
		case ACLUTIL_CXTYPE_VECTOR:
		case ACLUTIL_CXTYPE_ARRAY:
		case ACLUTIL_CXTYPE_FUNC_NOPRM:
		case ACLUTIL_CXTYPE_STRUCT_SINGLE:
		default:
			return !memcmp(&A->m_info, &B->m_info, sizeof(A->m_info));

		case ACLUTIL_CXTYPE_UNIT:
			break;
	}

	return 0;
}

ae2f_inline_f static aclspv_wrdcount_t s_find_node(
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

ae2f_inline_f static struct __ret {
	aclspv_id_t		m_id;
	aclspv_bool_t		m_isfirst;
} s_ret(const h_aclspv_ctx_t h_ctx, const aclutil_cxtp* ae2f_restrict rd_cxtp)
{
	aclspv_wrdcount_t	IDX = s_find_node(rd_cxtp, h_ctx);
	struct	__ret		RET;

#define	_return(c_id, c_is_first)	{ RET.m_id = c_id; RET.m_isfirst = c_is_first; return RET; }

	ae2f_ifezerr_strict(IDX == 0xFFFFFFFF)
		_return(__get_type_uniques(h_ctx)[IDX].m_type_id, 0);

	IDX = h_ctx->m_num_type_uniques;

	{
		size_t NEWSIZE = (h_ctx)->m_type_uniques.m_sz;
		while((IDX) >= NEWSIZE / sizeof(aclutil_cxtp) && (NEWSIZE <<= 1));
		ae2f_ifezerr(NEWSIZE) _return(0, 1);

		_aclspv_grow_vec_with_copy(_aclspv_malloc, _aclspv_free, _aclspv_memcpy
				, L_new, h_ctx->m_type_uniques, NEWSIZE);

		ae2f_ifezerr(h_ctx->m_type_uniques.m_p) {
			h_ctx->m_err = ACLSPV_COMPILE_ALLOC_FAILED_CXTP;
			err_puts("[cxtp::s_ret] allocation has failed.");
			_return(0, 1);
		}

	}

	(void)++h_ctx->m_num_type_uniques;

	{
		const aclspv_id_t	ID = h_ctx->m_id++;
		__get_type_uniques(h_ctx)[IDX] = *rd_cxtp;
		__get_type_uniques(h_ctx)[IDX].m_type_id = ID;
		_return(ID, 1);
	}

	ae2f_unreachable();

#undef	_return
}

#endif
