#include "./specop.h"
#include <util/wrdemit.h>
#include <.aclspv/util/vcopr.h>

static aclspv_id_t s_app(
		const aclspv_wrd_t* ae2f_restrict	rd_wrds,
		aclspv_wrdcount_t			c_num_wrd,
		h_aclspv_ctx_t				h_ctx
		) {
	const aclspv_wrd_t anchour = h_ctx->m_count.m_types;
	aclspv_wrd_t* ae2f_restrict wr;
	ae2f_assume(h_ctx);
	ae2f_ifezerr(h_ctx->m_count.m_types = aclutil_emit_wrd(
				&h_ctx->m_section.m_types
				, h_ctx->m_count.m_types + c_num_wrd - 1
				, 0
				)) {
		h_ctx->m_err = ACLSPV_COMPILE_MET_INVAL;
		return 0;
	}

	wr = &aclutil_get_wrd_of_vec(&h_ctx->m_section.m_types)[anchour];
	_aclutil_vcopr(L, =, wr, rd_wrds, c_num_wrd);

	return wr[2] = h_ctx->m_id++;
}

static const ae2f_ccpure aclspv_wrd_t* s_cmp_advance(
		const aclspv_wrd_t* ae2f_restrict rd_existing,
		const aclspv_wrd_t* ae2f_restrict rd_new,
		aclspv_wrd_t			c_num_new
		) {
	const aclspv_wrd_t* const R = rd_existing;

	unless(R[0] == rd_new[0]) {
		return R + aclspv_get_oprnd_count_for_opcode(R[0]);
	}

	unless(R[1] == rd_new[1]) {
		return R + aclspv_get_oprnd_count_for_opcode(R[0]);
	}

	ae2f_ifnezerr_strict(c_num_new < 3)
		return R + aclspv_get_oprnd_count_for_opcode(R[0]);

	c_num_new	-= 3;
	rd_existing	+= 3;
	rd_new		+= 3;

	while(c_num_new --> 0) {
		if((*rd_existing++) != (*rd_new++))
			return R + aclspv_get_oprnd_count_for_opcode(R[0]);
	}

	return R;
}

aclspv_id_t	aclconst_specop(
		const aclspv_wrd_t* ae2f_restrict	rd_wrds,
		aclspv_wrdcount_t			c_num_wrd,
		h_aclspv_ctx_t				h_ctx
		) {
	const aclspv_wrd_t* ae2f_restrict pool
		= aclutil_get_wrd_of_vec(&h_ctx->m_section.m_types);

	const aclspv_wrd_t* anchour, * pool0 = pool;

	while(
			(anchour = s_cmp_advance(pool, rd_wrds, c_num_wrd)) != pool
			&& anchour - pool0 < h_ctx->m_count.m_types)
		pool = anchour;

	if(anchour - pool0 >= h_ctx->m_count.m_types)
		return s_app(rd_wrds, c_num_wrd, h_ctx);

	return pool[2];
}
