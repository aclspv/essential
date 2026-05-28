#include <aclspv.h>
#include <util/scale.h>

#include "./impl/conf.h"

ACLSPV_ABI_IMPL void aclspv_fini_ctx(
		const h_aclspv_ctx_t	d_ctx
		) {
	enum ACLSPV_COMPILE_	E = d_ctx->m_err;

	_aclspv_stop_vec(_aclspv_free, d_ctx->m_constant_cache);
	_aclspv_stop_vec(_aclspv_free, d_ctx->m_type_uniques);
	_aclspv_stop_vec(_aclspv_free, d_ctx->m_cursors);
	_aclspv_stop_vec(_aclspv_free, d_ctx->m_scale_vars);

	_aclspv_stop_vec(_aclspv_free, d_ctx->m_fnlist.m_entp);
	_aclspv_stop_vec(_aclspv_free, d_ctx->m_fnlist.m_fn);

	_aclspv_stop_vec(_aclspv_free, d_ctx->m_tmp.m_v0);
	_aclspv_stop_vec(_aclspv_free, d_ctx->m_tmp.m_v1);
	_aclspv_stop_vec(_aclspv_free, d_ctx->m_tmp.m_v2);
	_aclspv_stop_vec(_aclspv_free, d_ctx->m_tmp.m_v3);

	_aclspv_stop_vec(_aclspv_free, d_ctx->m_section.m_capability);
	_aclspv_stop_vec(_aclspv_free, d_ctx->m_section.m_decorate);
	_aclspv_stop_vec(_aclspv_free, d_ctx->m_section.m_entp);
	_aclspv_stop_vec(_aclspv_free, d_ctx->m_section.m_fndef);
	_aclspv_stop_vec(_aclspv_free, d_ctx->m_section.m_execmode);
	_aclspv_stop_vec(_aclspv_free, d_ctx->m_section.m_ext);
	_aclspv_stop_vec(_aclspv_free, d_ctx->m_section.m_memmodel);
	_aclspv_stop_vec(_aclspv_free, d_ctx->m_section.m_name);
	_aclspv_stop_vec(_aclspv_free, d_ctx->m_section.m_types);
	_aclspv_stop_vec(_aclspv_free, d_ctx->m_section.m_vars);
	_aclspv_stop_vec(_aclspv_free, d_ctx->m_section.m_fnimpl);

	memset(d_ctx, 0, sizeof(x_aclspv_ctx));
	d_ctx->m_err = E;
}

ACLSPV_ABI_IMPL enum ACLSPV_COMPILE_ aclspv_init_ctx(
		const h_aclspv_ctx_t	i_ctx,
		const aclspv_wrd_t	c_init_size_exp,
		const aclspv_wrd_t	c_init_flag0,
		const aclspv_wrd_t	c_spirv_ver
		) {
	ae2f_ifezerr(i_ctx)
		return ACLSPV_COMPILE_PRM_INVAL_NIL;

	memset(i_ctx, 0, sizeof(x_aclspv_ctx));
	i_ctx->m_id		= ID_DEFAULT_END;
	i_ctx->m_flag0		|= c_init_flag0;
	i_ctx->m_spirv_ver	|= c_spirv_ver;

#define	ISZEXP2	ae2f_bll_sel_bool_naive1(size_t, c_init_size_exp, c_init_size_exp, 10)
#define	INITSZ	((size_t)(1 << ISZEXP2))

	_aclspv_grow_vec(_aclspv_malloc, _aclspv_free, i_ctx->m_constant_cache		, INITSZ);
	_aclspv_grow_vec(_aclspv_malloc, _aclspv_free, i_ctx->m_type_uniques		, INITSZ);
	_aclspv_grow_vec(_aclspv_malloc, _aclspv_free, i_ctx->m_scale_vars		, INITSZ);
	_aclspv_grow_vec(_aclspv_malloc, _aclspv_free, i_ctx->m_cursors			, INITSZ);

	_aclspv_grow_vec(_aclspv_malloc, _aclspv_free, i_ctx->m_fnlist.m_fn		, INITSZ);
	_aclspv_grow_vec(_aclspv_malloc, _aclspv_free, i_ctx->m_fnlist.m_entp		, INITSZ);

	_aclspv_grow_vec(_aclspv_malloc, _aclspv_free, i_ctx->m_section.m_capability	, INITSZ);
	_aclspv_grow_vec(_aclspv_malloc, _aclspv_free, i_ctx->m_section.m_entp		, INITSZ);
	_aclspv_grow_vec(_aclspv_malloc, _aclspv_free, i_ctx->m_section.m_ext		, INITSZ);
	_aclspv_grow_vec(_aclspv_malloc, _aclspv_free, i_ctx->m_section.m_vars		, INITSZ);
	_aclspv_grow_vec(_aclspv_malloc, _aclspv_free, i_ctx->m_section.m_decorate	, INITSZ);
	_aclspv_grow_vec(_aclspv_malloc, _aclspv_free, i_ctx->m_section.m_execmode	, INITSZ);
	_aclspv_grow_vec(_aclspv_malloc, _aclspv_free, i_ctx->m_section.m_memmodel	, INITSZ);
	_aclspv_grow_vec(_aclspv_malloc, _aclspv_free, i_ctx->m_section.m_fndef		, INITSZ);
	_aclspv_grow_vec(_aclspv_malloc, _aclspv_free, i_ctx->m_section.m_types		, INITSZ);

	_aclspv_grow_vec(_aclspv_malloc, _aclspv_free, i_ctx->m_tmp.m_v0		, INITSZ);
	_aclspv_grow_vec(_aclspv_malloc, _aclspv_free, i_ctx->m_tmp.m_v1		, INITSZ);
	_aclspv_grow_vec(_aclspv_malloc, _aclspv_free, i_ctx->m_tmp.m_v2		, INITSZ);
	_aclspv_grow_vec(_aclspv_malloc, _aclspv_free, i_ctx->m_tmp.m_v3		, INITSZ);

#undef	ISZEXP2

	ae2f_ifezerr(init_scale(&i_ctx->m_scale_vars, 0)) {
		aclspv_fini_ctx(i_ctx);
		return i_ctx->m_err = ACLSPV_COMPILE_ALLOC_FAILED;
	}

	return ae2f_unexpected(i_ctx->m_err)
		? i_ctx->m_err
		: (i_ctx->m_err = impl_conf(i_ctx));
}
