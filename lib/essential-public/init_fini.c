#include <aclspv.h>
#include <aclmem.h>

#include <util/scale.h>
#include <ae2f/cc/arg_sarr.h>
#include <aclctx/err.h>

/** dispose a parked-scratch stack: every element still on it owns a buffer. */
static void s_fini_vecstck(x_aclspv_vec* ae2f_restrict stck, aclspv_wrd_t* ae2f_restrict num) {
	x_aclspv_vec* ae2f_restrict base = (x_aclspv_vec* ae2f_restrict)stck->m_p;

	while(*num) {
		(void)--*num;
		aclmem_fini(&base[*num]);
	}

	aclmem_fini(stck);
	*num = 0;
}

static void s_fini_vectors(h_aclspv_ctx_t d) {
	aclmem_fini(&d->m_constant_cache);
	aclmem_fini(&d->m_type_uniques);
	aclmem_fini(&d->m_cursors);

	aclmem_fini(&d->m_cur_lbl_dfs);
	aclmem_fini(&d->m_cur_lbl_dfs_visit);

	aclmem_fini(&d->m_fnlist.m_entp);
	aclmem_fini(&d->m_fnlist.m_fn);
	aclmem_fini(&d->m_fnlist.m_fn_name);

	aclmem_fini(&d->m_tmp.m_v0);
	aclmem_fini(&d->m_body.m_bootscale);
	aclmem_fini(&d->m_body.m_cmdcountstck);

	s_fini_vecstck(&d->m_body.m_bootscale_stck, &d->m_body.m_num_bootscale_stck);
	s_fini_vecstck(&d->m_body.m_cmdcountstck_stck, &d->m_body.m_num_cmdcountstck_stck);

	aclmem_fini(&d->m_section.m_capability);
	aclmem_fini(&d->m_section.m_decorate);
	aclmem_fini(&d->m_section.m_entp);
	aclmem_fini(&d->m_section.m_fndef);
	aclmem_fini(&d->m_section.m_execmode);
	aclmem_fini(&d->m_section.m_ext);
	aclmem_fini(&d->m_section.m_extinst);
	aclmem_fini(&d->m_section.m_name);
	aclmem_fini(&d->m_section.m_name_symbol);
	aclmem_fini(&d->m_section.m_types);
	aclmem_fini(&d->m_section.m_vars);
	aclmem_fini(&d->m_section.m_fnimpl);
	return;
}

ACLSPV_ABI_LINK void aclspv_fini_ctx(
		const h_aclspv_ctx_t	d_ctx
		) {
	aclspv_compile_t	E = d_ctx->m_err;

	if(d_ctx->m_ctx_slave) {
		s_fini_vectors(d_ctx->m_ctx_slave);
		d_ctx->m_ctx_slave->m_usrcfg.m_free(
				d_ctx->m_ctx_slave->m_ctx_base
				, d_ctx->m_ctx_slave->m_ctx_sz
				, d_ctx->m_ctx_slave->m_usrcfg.m_usrdata);
	}

	s_fini_vectors(d_ctx);
	if(!E && d_ctx->m_err) E = d_ctx->m_err;

	aclmemzero(d_ctx, sizeof(x_aclspv_ctx));
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

static ae2f_inline_f ae2f_ccconst aclspv_usrcfg_t s_setup_usrcfg(aclspv_usrcfg_t c_usrcfg) {
	unless(c_usrcfg.m_max_push_size_limit)
		c_usrcfg.m_max_push_size_limit = 0x1000;

	unless(c_usrcfg.m_byte_size_type)
		c_usrcfg.m_byte_size_type = 4;

	unless(c_usrcfg.m_ver_spv)
		c_usrcfg.m_ver_spv = aclspv_ver_spv(1, 0);

	unless(c_usrcfg.m_ver_vk)
		c_usrcfg.m_ver_vk = aclspv_ver_vk2(1, 0);

	unless(c_usrcfg.m_malloc && c_usrcfg.m_free) {
		c_usrcfg.m_malloc	= s_malloc;
		c_usrcfg.m_free	= s_free;
	}

	unless(c_usrcfg.m_mdl_mem)
		c_usrcfg.m_mdl_mem = SpvMemoryModelSimple;

	unless(c_usrcfg.m_mdl_addr)
		c_usrcfg.m_mdl_addr = SpvAddressingModelLogical;

	return c_usrcfg;
}

ae2f_inline_f static x_aclspv_ctx* s_ctx_malloc(const aclspv_usrcfg_t ae2fcc_arg_sarr_restrict(rd_usrcfg, 1)) {
	x_aclspv_ctx*	ctx;
	void*		base;
	aclspv_wrd_t sz_real;

	ctx = rd_usrcfg->m_malloc(
			sizeof(x_aclspv_ctx)
			, &sz_real
			, &base
			, rd_usrcfg->m_usrdata
			);

	ae2f_ifezerr(ctx) return 0;

	ctx->m_ctx_sz = sz_real;
	ctx->m_ctx_base = base;
	return ctx;
}

ACLSPV_ABI_LINK void aclspv_ctx_free(x_aclspv_ctx* d) { if(d) {
	aclspv_usrcfg_t cfg = d->m_usrcfg;
	aclspv_fini_ctx(d);
	if(d->m_ctx_base) cfg.m_free(d->m_ctx_base, d->m_ctx_sz, cfg.m_usrdata);
}}

static ae2f_inline_f aclspv_compile_t s_init_vectors(h_aclspv_ctx_t i_ctx) {
	aclmem_init(&i_ctx->m_constant_cache		, i_ctx);
	aclmem_init(&i_ctx->m_type_uniques		, i_ctx);
	aclmem_init(&i_ctx->m_cursors			, i_ctx);

	aclmem_init(&i_ctx->m_cur_lbl_dfs			, i_ctx);
	aclmem_init(&i_ctx->m_cur_lbl_dfs_visit		, i_ctx);

	aclmem_init(&i_ctx->m_fnlist.m_fn			, i_ctx);
	aclmem_init(&i_ctx->m_fnlist.m_entp		, i_ctx);
	aclmem_init(&i_ctx->m_fnlist.m_fn_name		, i_ctx);

	aclmem_init(&i_ctx->m_section.m_capability	, i_ctx);
	aclmem_init(&i_ctx->m_section.m_entp		, i_ctx);
	aclmem_init(&i_ctx->m_section.m_ext		, i_ctx);
	aclmem_init(&i_ctx->m_section.m_extinst		, i_ctx);
	aclmem_init(&i_ctx->m_section.m_vars		, i_ctx);
	aclmem_init(&i_ctx->m_section.m_execmode		, i_ctx);
	aclmem_init(&i_ctx->m_section.m_fndef		, i_ctx);
	aclmem_init(&i_ctx->m_section.m_fnimpl		, i_ctx);
	aclmem_init(&i_ctx->m_section.m_types		, i_ctx);
	aclmem_init(&i_ctx->m_section.m_decorate		, i_ctx);
	aclmem_init(&i_ctx->m_section.m_name		, i_ctx);
	aclmem_init(&i_ctx->m_section.m_name_symbol	, i_ctx);

	aclmem_init(&i_ctx->m_tmp.m_v0			, i_ctx);
	aclmem_init(&i_ctx->m_body.m_bootscale			, i_ctx);
	aclmem_init(&i_ctx->m_body.m_cmdcountstck			, i_ctx);
	aclmem_init(&i_ctx->m_body.m_bootscale_stck		, i_ctx);
	aclmem_init(&i_ctx->m_body.m_cmdcountstck_stck		, i_ctx);
	i_ctx->m_body.m_num_bootscale_stck		= 0;
	i_ctx->m_body.m_num_cmdcountstck_stck		= 0;

	return i_ctx->m_err;
}

ACLSPV_ABI_LINK aclspv_compile_t aclspv_init_ctx(
		const h_aclspv_ctx_t	i_ctx,
		const aclspv_usrcfg_t*	ae2f_restrict	rd_usrcfg
		) {
	ae2f_ifezerr(i_ctx)
		return ACLSPV_COMPILE_PRM_INVAL_NIL;

	{
		void*		base = i_ctx->m_ctx_base;
		aclspv_wrd_t	sz_real = i_ctx->m_ctx_sz;

		aclmemzero(i_ctx, sizeof(*i_ctx));

		i_ctx->m_ctx_base = base;
		i_ctx->m_ctx_sz = sz_real;
	}

	rd_usrcfg ? (void)(i_ctx->m_usrcfg = *rd_usrcfg) : (void)0;
	i_ctx->m_usrcfg		= s_setup_usrcfg(i_ctx->m_usrcfg);
	i_ctx->m_id		= ACLSPV_ID_END;

	ae2f_ifnezerr(s_init_vectors(i_ctx))
		return i_ctx->m_err;

	ae2f_ifezerr(i_ctx->m_ctx_slave = s_ctx_malloc(&i_ctx->m_usrcfg)) {
		_aclctx_err(i_ctx, ACLSPV_COMPILE_ALLOC_FAILED);
		return i_ctx->m_err;
	}

	aclmemzero(i_ctx->m_ctx_slave, sizeof(*i_ctx->m_ctx_slave));
	i_ctx->m_ctx_slave->m_usrcfg = i_ctx->m_usrcfg;
	/** the same id base as the main context: a sub-context builds real
	 * type ids, and 0 is not a valid id. */
	i_ctx->m_ctx_slave->m_id = ACLSPV_ID_END;

	ae2f_ifezerr(s_init_vectors(i_ctx->m_ctx_slave))
		return i_ctx->m_err = i_ctx->m_ctx_slave->m_err;

	return i_ctx->m_err;
}

ACLSPV_ABI_LINK ae2f_retnew1(aclspv_ctx_free) x_aclspv_ctx* aclspv_ctx_make(const aclspv_usrcfg_t* ae2f_restrict rd_usrcfg) {
	aclspv_usrcfg_t usrcfg;
	aclmemzero(&usrcfg, sizeof(aclspv_usrcfg_t));
	if(rd_usrcfg) usrcfg = *rd_usrcfg;
	usrcfg = s_setup_usrcfg(usrcfg);

	{
		x_aclspv_ctx* const ctx = s_ctx_malloc(&usrcfg);
		ae2f_ifezerr(ctx) return 0;
		ctx->m_err = aclspv_init_ctx(ctx, &usrcfg);
		return ctx;
	}
}
