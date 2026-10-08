#include "./vec.h"
#include "./implutil.h"
#include "../aclty.h"

#include <util/log.h>
#include <util/iddef.h>
#include <util/emitx.h>
#include <ae2f/pat/dbg.h>

#include <aclmem.h>

#include <ae2f/cc/pause.h>

/** 1 when same */
ae2f_inline_f ae2f_ccpure static aclspv_bool_t s_ndeq(
		const aclty_t* const ae2f_restrict A,
		const aclty_t* const ae2f_restrict B
		)
{
	if(B->m_class != ACLTY_SIMD) return 0;

	if(B->m_info.m_vec.m_num != A->m_info.m_vec.m_num)
		return 0;

	if(B->m_info.m_vec.m_type != A->m_info.m_vec.m_type)
		return 0;

	return 1;
}

ae2f_inline_f ae2f_ccpure static aclspv_wrdcount_t s_find_node(
		const aclty_t* const ae2f_restrict rd_tar,
		const h_aclspv_ctx_t			h_ctx
		) {
	aclspv_wrdcount_t IDX = (h_ctx)->m_num_type_uniques;
	ae2f_ifezerr(h_ctx->m_type_uniques.m_p)
		return 0xFFFFFFFF;

	ae2f_ifnezerr((h_ctx)->m_num_type_uniques > (aclspv_wrd_t)(h_ctx->m_type_uniques.m_sz / sizeof(aclty_t)))
		return 0xFFFFFFFF;

	while(IDX-- 
			&& !s_ndeq(
				rd_tar
				, ((const aclty_t* ae2f_restrict)h_ctx->m_type_uniques.m_p) + IDX))
	{ ae2fcc_pause(); }

	return IDX;
}

ae2f_inline_f static struct __ret s_ret(const h_aclspv_ctx_t h_ctx, const aclty_t* ae2f_restrict rd_cxtp)
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

struct aclutil_cxtp_veccfg_t	aclty_get_veccfg(
		const aclspv_id_t	c_ty_vec,
		const h_aclspv_ctx_t	h_ctx
		) {
	aclspv_wrd_t			IDX		= h_ctx->m_num_type_uniques;
	const aclty_t* const ae2f_restrict P	= h_ctx->m_type_uniques.m_p;

	ae2fpat_assertassume(h_ctx);
	ae2fpat_assertassume(P);

	ae2f_ifezerr(c_ty_vec) {
		struct aclutil_cxtp_veccfg_t	R = {0, 0};
		return R;
	}

	if(aclid_is_vectorisable(c_ty_vec)) {
		struct aclutil_cxtp_veccfg_t	R;
		R.m_num		= 1;
		R.m_ty_elm	= c_ty_vec;

		return R;
	}

	while(IDX--
			&& ((P[IDX].m_class != ACLTY_SIMD) || P[IDX].m_type_id != c_ty_vec)) 
	{}

	dbg_prefix(a);
	dbg_call((void), (fprintf(stderr, "[cxtp::veccfg::%u/%u] WANTED %u\n"
					, IDX, h_ctx->m_num_type_uniques
					, c_ty_vec)));

	ae2f_ifneznrm(IDX < h_ctx->m_num_type_uniques) {
		struct aclutil_cxtp_veccfg_t	R;
		R.m_num		= P[IDX].m_info.m_vec.m_num;
		R.m_ty_elm	= P[IDX].m_info.m_vec.m_type;

		return R;
	} else {
		struct aclutil_cxtp_veccfg_t	R = {0, 0};
		return R;
	}

	ae2f_unreachable();
}

#include <aclconst/align.h>

#include <assert.h>

aclspv_id_t aclty_mk_vec(
		const aclspv_id_t       c_type_elm,
		const aclspv_wrd_t      c_num_arr,
		const h_aclspv_ctx_t    h_ctx
		)
{
	aclty_t            TAR;
	ae2f_ifnezerr(h_ctx->m_err)	return 0;
	ae2f_ifezerr(c_num_arr)		return 0;

	aclmemzero(&TAR, sizeof(TAR));

	TAR.m_class = ACLTY_SIMD;
	TAR.m_info.m_vec.m_num = c_num_arr;
	TAR.m_info.m_vec.m_type = c_type_elm;
	TAR.m_alignof		= aclconst_align_from_size2(
			aclty_getalign(c_type_elm, h_ctx) * c_num_arr
			, h_ctx
			).m_lit.m_wrd;

	TAR.m_sizeof.m_lit.m_wrd	= TAR.m_alignof;
	TAR.m_sizeof.m_id		= 0;

	assert(aclconst_align_from_size2(c_num_arr, h_ctx).m_lit.m_wrd);
	assert(aclconst_align_from_size2(8 * c_num_arr, h_ctx).m_lit.m_wrd);

	ae2f_ifezerr(TAR.m_alignof) {
		err_prefix(a);
		err_call(fprintf, (stderr
					, "TAR.m_alignof is zero! vector( getalign: %lu, c_num_arr: %lu) -> %lu\n"
					, (unsigned long)aclty_getalign(c_type_elm, h_ctx)
					, (unsigned long)c_num_arr
					, (unsigned long)aclconst_align_from_size2(aclty_getalign(c_type_elm, h_ctx) * c_num_arr, h_ctx).m_lit.m_max
					));
		return 0;
	}

	/** when 0, 1 arrnum is no needed or not possible */
	unless(c_num_arr >> 1) {
		return c_type_elm;
	}

	(void)(aclid_mk_default_id(c_type_elm, h_ctx));

	{
		const struct __ret ID = s_ret(h_ctx, &TAR); 
		ae2f_ifezerr(ID.m_id)		return 0;
		unless(ID.m_isfirst)            return ID.m_id;

		ae2f_ifezerr(h_ctx->m_count.m_types = aclutil_emitx_4(
					&h_ctx->m_section.m_types
					, h_ctx->m_count.m_types
					, SpvOpTypeVector
					, ID.m_id
					, c_type_elm
					, c_num_arr
					)) return 0;

		dbg_prefix();
		dbg_call((void), (fprintf(stderr
						, "[cxtp] made a vector [%u]\n"
						, ID.m_id)));

		return ID.m_id;
	}
}
