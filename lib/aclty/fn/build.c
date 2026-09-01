#include "./build.h"
#include "../implutil.h"
#include "../dcr.h"
#include "./arg.h"
#include <util/wrdemit.h>

/*** helper: are two function types equivalent? */
static ae2f_ccpure aclspv_bool_t	s_is_fn_same(
		const aclutil_cxtp* ae2f_restrict rd0,
		const aclutil_cxtp* ae2f_restrict rd1,
		h_aclspv_ctx_t h_ctx)
{
	if(rd0 == rd1) return 0;
	if(rd0->m_class != rd1->m_class) return 0;

	/** ACLUTIL_CXTYPE_FUNC */
	if(rd0->m_info.m_fn.m_ret_id != rd1->m_info.m_fn.m_ret_id)
		return 0;

	if(rd0->m_info.m_fn.m_num_arg != rd1->m_info.m_fn.m_num_arg)
		return 0;

	if(rd0->m_info.m_fn.m_num_dcr != rd1->m_info.m_fn.m_num_dcr)
		return 0;

	if(rd0->m_info.m_fn.m_dcr && !aclty_is_dcr_same(
			&__get_type_uniques(h_ctx)[rd0->m_info.m_fn.m_dcr]
			, &__get_type_uniques(h_ctx)[rd1->m_info.m_fn.m_dcr]
			, h_ctx)) return 0;

	if(!rd0->m_info.m_fn.m_num_arg)
		return 1;

	{
		aclspv_wrd_t	ia = rd0->m_info.m_fn.m_num_arg;
		const aclutil_cxtp* a0 = &__get_type_uniques(h_ctx)[rd0->m_info.m_fn.m_arg];
		const aclutil_cxtp* a1 = &__get_type_uniques(h_ctx)[rd1->m_info.m_fn.m_arg];

		while(ia --> 0) {
			if(!a0 || !a1) return 0;

			if(a0->m_class != ACLUTIL_CXTYPE_FUNC_ARG
			|| a1->m_class != ACLUTIL_CXTYPE_FUNC_ARG)
				return 0;

			if(a0->m_info.m_fn_arg.m_prm_id
			!= a1->m_info.m_fn_arg.m_prm_id)
				return 0;

			a0 = a0->m_info.m_fn_arg.m_nxt
				? &__get_type_uniques(h_ctx)[a0->m_info.m_fn_arg.m_nxt] : 0;
			a1 = a1->m_info.m_fn_arg.m_nxt
				? &__get_type_uniques(h_ctx)[a1->m_info.m_fn_arg.m_nxt] : 0;
		}
	}

	return 1;
}

static void s_free_fn(aclutil_cxtp* fini, h_aclspv_ctx_t h_ctx) {
	if(fini->m_info.m_fn.m_dcr) {
		aclty_free_dcr(
				&__get_type_uniques(h_ctx)[fini->m_info.m_fn.m_dcr]
				, h_ctx
				);
	}

	fini->m_class = ACLUTIL_CXTYPE_RESERVED;
}

aclspv_id_t aclty_build_fn(
		aclutil_cxtp* ae2f_restrict rdwr_fn,
		h_aclspv_ctx_t h_ctx)
{
	aclspv_wrd_t	idx, pad, num_arg;
	aclspv_id_t	ret_id;

	ae2f_ifezerr(rdwr_fn) return 0;

	if(rdwr_fn->m_class != ACLUTIL_CXTYPE_FUNC)
		return 0;

	ret_id	= rdwr_fn->m_info.m_fn.m_ret_id;
	num_arg	= rdwr_fn->m_info.m_fn.m_num_arg;

	/*** dedup against already configured / emitted types */
	{
		aclspv_wrd_t	i = h_ctx->m_num_type_uniques;

		while(i --> 0) {
			aclutil_cxtp* cxtp2 = &__get_type_uniques(h_ctx)[i];

			if(s_is_fn_same(rdwr_fn, cxtp2, h_ctx)) {
				s_free_fn(rdwr_fn, h_ctx);
				return cxtp2->m_type_id;
			}
		}
	}

	/*** emit SpvOpTypeFunction (opcode, result id, return id, arg ids) */
	{
		idx	= num_arg;
		pad	= h_ctx->m_count.m_types;

		ae2f_ifezerr(h_ctx->m_count.m_types = aclutil_emit_wrd(
					&h_ctx->m_section.m_types
					, h_ctx->m_count.m_types + num_arg + 2
					, 0
					)) {
			return 0;
		}

		while(idx --> 0) {
			aclutil_cxtp* arg = aclty_get_fn_arg(rdwr_fn, idx, h_ctx);
			ae2f_ifezerr(arg) return 0;

			aclutil_get_wrd_of_vec(&h_ctx->m_section.m_types)[pad + idx + 3]
				= arg->m_info.m_fn_arg.m_prm_id;
		}

		aclutil_get_wrd_of_vec(&h_ctx->m_section.m_types)[pad + 0]
			= aclspv_opcode_to_wrd(SpvOpTypeFunction, num_arg + 2);
		aclutil_get_wrd_of_vec(&h_ctx->m_section.m_types)[pad + 1]
			= rdwr_fn->m_type_id;
		aclutil_get_wrd_of_vec(&h_ctx->m_section.m_types)[pad + 2]
			= ret_id;
	}

	/*** build decorations */
	if(rdwr_fn->m_info.m_fn.m_dcr) {
		ae2f_ifnezerr_strict(aclty_build_dcr(
					&__get_type_uniques(h_ctx)[rdwr_fn->m_info.m_fn.m_dcr]
					, rdwr_fn->m_type_id
					, h_ctx)) return 0;
	}

	return rdwr_fn->m_type_id;
}
