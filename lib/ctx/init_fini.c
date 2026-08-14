#include <aclspv.h>
#include <aclmem.h>

#include <util/scale.h>
#include <ae2f/cc/memset.h>

#include "./conf.h"

ACLSPV_ABI_IMPL void aclspv_fini_ctx(
		const h_aclspv_ctx_t	d_ctx
		) {
	enum ACLSPV_COMPILE_	E = d_ctx->m_err;


	aclmem_fini(&d_ctx->m_constant_cache);
	aclmem_fini(&d_ctx->m_type_uniques);
	aclmem_fini(&d_ctx->m_cursors);

	aclmem_fini(&d_ctx->m_fnlist.m_entp);
	aclmem_fini(&d_ctx->m_fnlist.m_fn);

	aclmem_fini(&d_ctx->m_tmp.m_v0);
	aclmem_fini(&d_ctx->m_tmp.m_v1);
	aclmem_fini(&d_ctx->m_tmp.m_v2);

	aclmem_fini(&d_ctx->m_section.m_capability);
	aclmem_fini(&d_ctx->m_section.m_decorate);
	aclmem_fini(&d_ctx->m_section.m_entp);
	aclmem_fini(&d_ctx->m_section.m_fndef);
	aclmem_fini(&d_ctx->m_section.m_execmode);
	aclmem_fini(&d_ctx->m_section.m_ext);
	aclmem_fini(&d_ctx->m_section.m_memmodel);
	aclmem_fini(&d_ctx->m_section.m_name);
	aclmem_fini(&d_ctx->m_section.m_types);
	aclmem_fini(&d_ctx->m_section.m_vars);
	aclmem_fini(&d_ctx->m_section.m_fnimpl);

	ae2fcc_memset(d_ctx, 0, sizeof(x_aclspv_ctx));
	d_ctx->m_err = E;
}

#include <aclemit/root.h>
#include <ae2f/Sys/pgalloc.h>
#include <util/log.h>

static void* s_malloc(aclspv_wrd_t c_sz,
			aclspv_wrd_t* ae2f_restrict r_sz_real,
			void* ae2f_restrict *	r_mem,
			ae2f_unused void*	h_usr) {
#ifdef	_ae2fsys_pgalloc_algnup

	void*	R;
	ae2fsys_pgsz_t	szreal;

	/** dbg_puts("[aclspv::malloc]"); */

	_ae2fsys_pgalloc_algnup(
			L, void*, R, 0, szreal
			, (ae2fsys_pgsz_t)c_sz
			, AE2FSYS_PGALLOC_PROT_WRITE | AE2FSYS_PGALLOC_PROT_READ
			, AE2FSYS_PGALLOC_FLAG_EASY
			);

	if(szreal & (~(ae2fsys_pgsz_t)0xFFFFFFFF)) {
		ae2f_unused enum AE2FSYS_PGALLOC_ e;
		_ae2fsys_pgfree(L, void*, e, R, szreal);
		R = AE2FSYS_PGALLOC_FAILED;
	}

	if(R == AE2FSYS_PGALLOC_FAILED) {
		*r_mem		= 0;
		*r_sz_real	= 0;
		return 0;
	}

	*r_sz_real = (aclspv_wrd_t)szreal;
	return (*r_mem = R);
#endif
}
static void s_free(void* fini_mem, aclspv_wrd_t c_sz, ae2f_unused void* h_usr) {
#ifdef	_ae2fsys_pgfree
	ae2f_unused enum AE2FSYS_PGALLOC_ e;

	/** dbg_puts("[aclspv::free]"); */
	if(fini_mem) _ae2fsys_pgfree(L, void*, e, fini_mem, c_sz);
#endif
}

ACLSPV_ABI_IMPL enum ACLSPV_COMPILE_ aclspv_init_ctx(
		const h_aclspv_ctx_t	i_ctx,
		const aclspv_usrcfg_t*	ae2f_restrict	rd_usrcfg
		) {
	ae2f_ifezerr(i_ctx)
		return ACLSPV_COMPILE_PRM_INVAL_NIL;

	ae2fcc_memset(i_ctx, 0, sizeof(x_aclspv_ctx));

	i_ctx->m_id		= ACLSPV_ID_END;
	i_ctx->m_work_nxt	= aclemit_root;
	i_ctx->m_work_data	= 0;

	rd_usrcfg ? (void)(i_ctx->m_usrcfg = *rd_usrcfg) : (void)0;

	unless(i_ctx->m_usrcfg.m_max_push_size_limit)
		i_ctx->m_usrcfg.m_max_push_size_limit = 0x1000;

	unless(i_ctx->m_usrcfg.m_byte_size_type)
		i_ctx->m_usrcfg.m_byte_size_type = 4;

	unless(i_ctx->m_usrcfg.m_spirv_ver)
		i_ctx->m_usrcfg.m_spirv_ver = aclspv_ver_spv(1, 0);

	unless(i_ctx->m_usrcfg.m_malloc && i_ctx->m_usrcfg.m_free) {
		i_ctx->m_usrcfg.m_malloc	= s_malloc;
		i_ctx->m_usrcfg.m_free		= s_free;
	}

	aclmem_init(&i_ctx->m_constant_cache		, i_ctx);
	aclmem_init(&i_ctx->m_type_uniques		, i_ctx);
	aclmem_init(&i_ctx->m_cursors			, i_ctx);

	aclmem_init(&i_ctx->m_fnlist.m_fn			, i_ctx);
	aclmem_init(&i_ctx->m_fnlist.m_entp		, i_ctx);

	aclmem_init(&i_ctx->m_section.m_capability	, i_ctx);
	aclmem_init(&i_ctx->m_section.m_entp		, i_ctx);
	aclmem_init(&i_ctx->m_section.m_ext		, i_ctx);
	aclmem_init(&i_ctx->m_section.m_vars		, i_ctx);
	aclmem_init(&i_ctx->m_section.m_execmode		, i_ctx);
	aclmem_init(&i_ctx->m_section.m_memmodel		, i_ctx);
	aclmem_init(&i_ctx->m_section.m_fndef		, i_ctx);
	aclmem_init(&i_ctx->m_section.m_fnimpl		, i_ctx);
	aclmem_init(&i_ctx->m_section.m_types		, i_ctx);
	aclmem_init(&i_ctx->m_section.m_decorate		, i_ctx);
	aclmem_init(&i_ctx->m_section.m_name		, i_ctx);

	aclmem_init(&i_ctx->m_tmp.m_v0			, i_ctx);
	aclmem_init(&i_ctx->m_tmp.m_v1			, i_ctx);
	aclmem_init(&i_ctx->m_tmp.m_v2			, i_ctx);


	return ae2f_unexpected(i_ctx->m_err)
		? i_ctx->m_err
		: (i_ctx->m_err = aclspv_ctx_conf(i_ctx));
}
