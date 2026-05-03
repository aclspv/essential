#include <util/cxtp.h>
#include <util/log.h>
#include <ae2f/pat/dbg.h>

struct aclutil_cxtp_veccfg_t	aclutil_cxtp_get_veccfg(
		const aclspv_id_t	c_ty_vec,
		const h_aclspv_ctx_t	h_ctx
		) {
	aclspv_wrd_t			IDX		= h_ctx->m_num_type_uniques;
	const aclutil_cxtp* const ae2f_restrict P	= h_ctx->m_type_uniques.m_p;

	ae2fpat_assertassume(h_ctx);
	ae2fpat_assertassume(P);

	while(IDX--
			&& ((P[IDX].m_class != ACLUTIL_CXTYPE_VECTOR) || P[IDX].m_type_id != c_ty_vec)) 
	{}

	dbg_prefix(a);
	dbg_call((void), (fprintf(stderr, "[cxtp::veccfg::%u/%u] WANTED %u\n"
					, IDX, h_ctx->m_num_type_uniques
					, c_ty_vec)));

	if(IDX < h_ctx->m_num_type_uniques) {
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
