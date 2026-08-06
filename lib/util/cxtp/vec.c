#include "./vec.h"
#include "./implutil.h"

#include <util/cxtp.h>
#include <util/log.h>
#include <util/iddef.h>
#include <util/emitx.h>
#include <ae2f/pat/dbg.h>

 struct aclutil_cxtp_veccfg_t	aclutil_cxtp_get_veccfg(
		const aclspv_id_t	c_ty_vec,
		const h_aclspv_ctx_t	h_ctx
		) {
	aclspv_wrd_t			IDX		= h_ctx->m_num_type_uniques;
	const aclutil_cxtp* const ae2f_restrict P	= h_ctx->m_type_uniques.m_p;

	ae2fpat_assertassume(h_ctx);
	ae2fpat_assertassume(P);

	if(aclid_is_vectorisable(c_ty_vec)) {
		struct aclutil_cxtp_veccfg_t	R;
		R.m_num		= 1;
		R.m_ty_elm	= c_ty_vec;

		return R;
	}

	while(IDX--
			&& ((P[IDX].m_class != ACLUTIL_CXTYPE_VECTOR) || P[IDX].m_type_id != c_ty_vec)) 
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

aclspv_id_t aclutil_mk_cxtp_vec(
		const aclspv_id_t       c_type_elm,
		const aclspv_wrd_t      c_num_arr,
		const h_aclspv_ctx_t    h_ctx
		)
{
	aclutil_cxtp            TAR;
	ae2f_ifnezerr(h_ctx->m_err) return 0;

	memset(&TAR, 0, sizeof(TAR));

	TAR.m_class = ACLUTIL_CXTYPE_VECTOR;
	TAR.m_info.m_vec.m_num = c_num_arr;
	TAR.m_info.m_vec.m_type = c_type_elm;
	TAR.m_alignof		= aclconst_align_from_size2(
			aclutil_cxtp_alignof(c_type_elm, h_ctx) * c_num_arr
			, h_ctx
			).m_id;

	TAR.m_sizeof.m_lit.m_wrd	= TAR.m_alignof;
	TAR.m_sizeof.m_id		= 0;

	/** when 0, 1 arrnum is no needed or not possible */
	unless(c_num_arr >> 1) {
		return c_type_elm;
	}

	(void)(aclid_mk_default_id(c_type_elm, h_ctx));

	{
		const struct __ret ID = s_ret(h_ctx, &TAR); 
		ae2f_expected_but_else(ID.m_id) return 0;
		unless(ID.m_isfirst)            return ID.m_id;

		ae2f_expected_but_else(h_ctx->m_count.m_types = aclutil_emitx_4(
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
