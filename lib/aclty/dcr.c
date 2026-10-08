#include "./dcr.h"
#include "./implutil.h"

#include <ae2f/pat/dbg.h>

ae2f_ccpure aclty_t* aclty_get_dcr(aclty_t* ae2f_restrict rd_dcr, aclspv_wrd_t c_decor, h_aclspv_ctx_t h_ctx) {
	ae2fpat_assertassumeunless(rd_dcr) {
		h_ctx->m_err = ACLSPV_COMPILE_CXTP_ZERO;
		return 0;
	}

	do {
		if(rd_dcr->m_info.m_dcr.m_op == (aclspv_wrd_t)c_decor)
			return rd_dcr;

		rd_dcr = rd_dcr->m_info.m_dcr.m_nxt
			? &__get_type_uniques(h_ctx)[(rd_dcr->m_info.m_dcr.m_nxt)]
			: 0;
	} while(rd_dcr);

	return 0;
}

ae2f_ccpure static aclspv_bool_t s_same_dcr_help(
		aclty_t* ae2f_restrict	rd0,
		aclty_t* ae2f_restrict	rd1,
		h_aclspv_ctx_t			h_ctx
		) {
	aclspv_wrd_t	i = MAX_ACLCXTP_DCR_OPRND;

	if(rd0 == rd1) return 1;
	if(rd0->m_class != rd1->m_class) return 0;

	if(rd0->m_info.m_dcr.m_num_oprnd != rd1->m_info.m_dcr.m_num_oprnd) return 0;
	if(rd0->m_info.m_dcr.m_op != rd1->m_info.m_dcr.m_op) return 0;

	while(i --> 0) {
		if(rd0->m_info.m_dcr.m_oprnd[i] != rd1->m_info.m_dcr.m_oprnd[i])
			return 0;
	}

	if(!!rd0->m_info.m_dcr.m_nxt != !!rd1->m_info.m_dcr.m_nxt)
		return 0;

	if(!rd0->m_info.m_dcr.m_nxt)
		return 1;

	rd0 = &__get_type_uniques(h_ctx)[rd0->m_info.m_dcr.m_nxt];
	rd1 = &__get_type_uniques(h_ctx)[rd1->m_info.m_dcr.m_nxt];

	return s_same_dcr_help(rd0, rd1, h_ctx);
}

ae2f_ccpure aclspv_bool_t aclty_is_dcr_same(
		aclty_t* ae2f_restrict rd0,
		aclty_t* ae2f_restrict rd1,
		h_aclspv_ctx_t		h_ctx
		) {
	return s_same_dcr_help(rd0, rd1, h_ctx);
}

void aclty_free_dcr(aclty_t* ae2f_restrict fini, h_aclspv_ctx_t h_ctx) {
	while(fini->m_class == ACLTY_DCR) {
		fini->m_class = ACLTY_RESERVED;

		if(fini->m_info.m_dcr.m_nxt)
			fini = &__get_type_uniques(h_ctx)[fini->m_info.m_dcr.m_nxt];

	}
}


#include <util/wrdemit.h>

aclspv_bool_t aclty_build_dcr(const aclty_t* ae2f_restrict rd, aclspv_id_t c_id, h_aclspv_ctx_t h_ctx) {
	while(rd->m_class == ACLTY_DCR) {
		aclspv_wrd_t	idx = rd->m_info.m_dcr.m_num_oprnd,
				pad = h_ctx->m_count.m_decorate;

		ae2f_ifezerr(h_ctx->m_count.m_decorate = aclutil_emit_wrd(
					&h_ctx->m_section.m_decorate
					, h_ctx->m_count.m_decorate + rd->m_info.m_dcr.m_num_oprnd + 2
					, 0
					)) {
			return 1;
		}

		while(idx --> 0) {
			((aclspv_wrd_t* ae2f_restrict)h_ctx->m_section.m_decorate.m_p)[pad + idx + 3]
				= rd->m_info.m_dcr.m_oprnd[idx];
		}


		((aclspv_wrd_t* ae2f_restrict)h_ctx->m_section.m_decorate.m_p)[pad + 0] = aclspv_opcode_to_wrd(SpvOpDecorate, rd->m_info.m_dcr.m_num_oprnd + 2);
		((aclspv_wrd_t* ae2f_restrict)h_ctx->m_section.m_decorate.m_p)[pad + 1] = c_id;
		((aclspv_wrd_t* ae2f_restrict)h_ctx->m_section.m_decorate.m_p)[pad + 2] = rd->m_info.m_dcr.m_op;

		rd = &__get_type_uniques(h_ctx)[rd->m_info.m_dcr.m_nxt];
	}

	return 0;
}
aclspv_bool_t aclty_build_dcr_mdcr(const aclty_t* ae2f_restrict rd, aclspv_id_t c_id, aclspv_wrd_t c_idx, h_aclspv_ctx_t h_ctx) {
	while(rd->m_class == ACLTY_DCR) {
		aclspv_wrd_t	idx = rd->m_info.m_dcr.m_num_oprnd,
				pad = h_ctx->m_count.m_decorate;

		ae2f_ifezerr(h_ctx->m_count.m_decorate = aclutil_emit_wrd(
					&h_ctx->m_section.m_decorate
					, pad + idx + 3
					, 0
					)) {
			return 1;
		}

		while(idx --> 0) {
			((aclspv_wrd_t* ae2f_restrict)h_ctx->m_section.m_decorate.m_p)[pad + idx + 4]
				= rd->m_info.m_dcr.m_oprnd[idx];
		}


		((aclspv_wrd_t* ae2f_restrict)h_ctx->m_section.m_decorate.m_p)[pad + 0] = aclspv_opcode_to_wrd(SpvOpMemberDecorate, rd->m_info.m_dcr.m_num_oprnd + 3);
		((aclspv_wrd_t* ae2f_restrict)h_ctx->m_section.m_decorate.m_p)[pad + 1] = c_id;
		((aclspv_wrd_t* ae2f_restrict)h_ctx->m_section.m_decorate.m_p)[pad + 2] = c_idx;
		((aclspv_wrd_t* ae2f_restrict)h_ctx->m_section.m_decorate.m_p)[pad + 3] = rd->m_info.m_dcr.m_op;
		rd = &__get_type_uniques(h_ctx)[rd->m_info.m_dcr.m_nxt];
	}

	return 0;
}
