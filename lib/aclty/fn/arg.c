#include "./arg.h"
#include "../implutil.h"

aclutil_cxtp* aclty_get_fn_arg(
		aclutil_cxtp* ae2f_restrict rdwr_fn,
		aclspv_wrd_t c_idx,
		h_aclspv_ctx_t h_ctx) {
	aclutil_cxtp* ae2f_restrict R;

	ae2f_ifnezerr_strict(rdwr_fn->m_info.m_fn.m_num_arg <= c_idx)
		return 0;

	R = &__get_type_uniques(h_ctx)[rdwr_fn->m_info.m_fn.m_arg];

	while(R->m_class == ACLUTIL_CXTYPE_FUNC_ARG)
	{
		if(R->m_info.m_fn_arg.m_idx == c_idx)
			return R;

		R = &__get_type_uniques(h_ctx)[(R->m_info.m_fn_arg.m_nxt)];
	}

	/** UNLIKELY */
	ae2f_ifnezerr_strict(1) {
		h_ctx->m_err = ACLSPV_COMPILE_CXTP_FN_ARGS_NUM_IS_LYING;
		return 0;
	} else {
		ae2f_unreachable();
	}
}

#include <aclty.h>

aclutil_cxtp* aclty_add_fn_arg(
		aclspv_id_t	c_type,
		h_aclspv_ctx_t	h_ctx,
		aclutil_cxtp* ae2f_restrict rdwr_fn
		) {
	typedef aclutil_cxtp* ae2f_restrict p_cxtp_t;

	aclutil_cxtp	cxtp;
	p_cxtp_t	new_p;
	struct __ret	node;

	ae2f_ifnezerr_strict(rdwr_fn->m_class != ACLUTIL_CXTYPE_FUNC)
		return 0;

	aclmemzero(&cxtp, sizeof(cxtp));
	cxtp.m_class = ACLUTIL_CXTYPE_FUNC_ARG;
	cxtp.m_info.m_fn_arg.m_idx	= rdwr_fn->m_info.m_fn.m_num_arg;
	cxtp.m_info.m_fn_arg.m_prm_id	= c_type;

	node = s_ret_new(h_ctx, &cxtp, 0, 0);
	ae2f_ifezerr(node.m_id)
		return 0;

	ae2f_ifezerr(new_p = aclty_find(node.m_id, h_ctx))
		return 0;

	/** have arg? */
	if(rdwr_fn->m_info.m_fn.m_num_arg) {
		p_cxtp_t tail_p = aclty_get_fn_arg(
				rdwr_fn
				, rdwr_fn->m_info.m_fn.m_num_arg - 1
				, h_ctx);

		ae2f_ifezerr(tail_p) return 0;
		tail_p->m_info.m_fn_arg.m_nxt = (aclspv_wrd_t)(new_p - &__get_type_uniques(h_ctx)[0]);
	} else {
		rdwr_fn->m_info.m_fn.m_arg = (aclspv_wrd_t)(new_p - &__get_type_uniques(h_ctx)[0]);
	}

	++rdwr_fn->m_info.m_fn.m_num_arg;
	return new_p;
}
