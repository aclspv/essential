#include "../implutil.h"

#include "../dcr.h"
#include "../dcr/util.h"

#include <util/cxtp.h>
#include <ae2f/cc/memset.h>
#include <ae2f/pat/dbg.h>

#include "./mbr.h"

/*** STRUCT MEMBER ***/

ae2f_ccpure static aclutil_cxtp* s_get_last_mbr(aclutil_cxtp* ae2f_restrict rd_mbr, h_aclspv_ctx_t h_ctx) {
	while(rd_mbr->m_info.m_struct_mbr.m_nxt)
	{
		rd_mbr = &__get_type_uniques(h_ctx)[(rd_mbr->m_info.m_struct_mbr.m_nxt)];
	}

	return rd_mbr;
}


aclutil_cxtp* aclutil_cxtp_struct_mbr_dcr_cfg2(aclspv_id_t c_id, SpvDecoration c_decor, h_aclspv_ctx_t h_ctx) {
	aclutil_cxtp* ae2f_restrict cxtp = aclutil_find_cxtp(c_id, h_ctx);
	return ae2f_expected(cxtp) ? aclutil_cxtp_struct_mbr_dcr_cfg(cxtp, c_decor, h_ctx) : 0;
}

ae2f_ccpure aclutil_cxtp* aclutil_cxtp_struct_get_mbr(
		aclutil_cxtp* ae2f_restrict rdwr_struct,
		aclspv_wrd_t c_idx,
		h_aclspv_ctx_t h_ctx) {
	aclutil_cxtp* ae2f_restrict R;

	ae2f_ifnezerr_strict(rdwr_struct->m_info.m_struct.m_num_mbr <= c_idx)
		return 0;

	R = &__get_type_uniques(h_ctx)[rdwr_struct->m_info.m_struct.m_mbr];

	while(R->m_class == ACLUTIL_CXTYPE_STRUCT_MBR)
	{
		if(R->m_info.m_struct_mbr.m_idx == c_idx)
			return R;

		R = &__get_type_uniques(h_ctx)[(R->m_info.m_struct_mbr.m_nxt)];
	}

	/** UNLIKELY */
	ae2f_ifnezerr_strict(1) {
		h_ctx->m_err = ACLSPV_COMPILE_CXTP_STRUCT_MBR_NUM_IS_LYING;
		return 0;
	} else {
		ae2f_unreachable();
	}
}

aclutil_cxtp* aclutil_cxtp_struct_add_mbr(aclutil_cxtp* ae2f_restrict rdwr_struct, aclspv_id_t c_id, h_aclspv_ctx_t h_ctx) {
	aclutil_cxtp* ae2f_restrict r, * ae2f_restrict r2;
	const aclspv_wrd_t	pad = (aclspv_wrd_t)(rdwr_struct - __get_type_uniques(h_ctx));

	ae2fpat_assertassumeunless(rdwr_struct) {
		h_ctx->m_err = ACLSPV_COMPILE_CXTP_ZERO;
		return 0;
	}

	ae2fpat_assertassumeunless(rdwr_struct->m_class == ACLUTIL_CXTYPE_STRUCT) {
		h_ctx->m_err = ACLSPV_COMPILE_CXTP_TYINVL_STRUCT_DCR;
		return 0;
	}

	{
		struct __ret	ID;
		aclutil_cxtp	TAR;

		ae2fcc_memset(&TAR, 0, sizeof(TAR));

		TAR.m_class				= ACLUTIL_CXTYPE_STRUCT_MBR;
		TAR.m_info.m_struct_mbr.m_id_mbr	= c_id;
		TAR.m_info.m_struct_mbr.m_idx		= rdwr_struct->m_info.m_struct.m_num_mbr++;

		ID = s_ret_new(h_ctx, &TAR, h_ctx->m_id++, 0);
		rdwr_struct = &__get_type_uniques(h_ctx)[pad];

		ae2f_ifezerr(ID.m_id) {
			(void)--rdwr_struct->m_info.m_struct.m_num_mbr;
			return 0;
		}

		r = aclutil_find_cxtp(ID.m_id, h_ctx);
		assert(r->m_type_id == ID.m_id);
	}

	if(!rdwr_struct->m_info.m_struct.m_mbr) {
		rdwr_struct->m_info.m_struct.m_mbr = (aclspv_wrd_t)(r - __get_type_uniques(h_ctx));
		return r;
	}

	r2 = s_get_last_mbr(&__get_type_uniques(h_ctx)[rdwr_struct->m_info.m_struct.m_mbr], h_ctx);
	r2->m_info.m_struct_mbr.m_nxt = (aclspv_wrd_t)(r - __get_type_uniques(h_ctx));

	ae2f_ifnezerr_strict(r2->m_info.m_struct_mbr.m_idx + 1 != r->m_info.m_struct_mbr.m_idx) {
		err_prefix(a);
		err_call((void), fprintf(stderr, "struct member num is lying: %u %u\n"
					, r2->m_info.m_struct_mbr.m_idx + 1
					, r->m_info.m_struct_mbr.m_idx));
		h_ctx->m_err = ACLSPV_COMPILE_CXTP_STRUCT_MBR_NUM_IS_LYING;
		return 0;
	}

	return r;
}


aclutil_cxtp* aclutil_cxtp_struct_mbr_dcr_cfg(aclutil_cxtp* ae2f_restrict rdwr_mbr, SpvDecoration c_decor, h_aclspv_ctx_t h_ctx) {
	aclutil_cxtp*	r = 0;
	aclspv_wrd_t pad = (aclspv_wrd_t)(rdwr_mbr - __get_type_uniques(h_ctx));

	ae2fpat_assertassumeunless(rdwr_mbr) {
		h_ctx->m_err = ACLSPV_COMPILE_CXTP_ZERO;
		return 0;
	}

	ae2fpat_assertassumeunless(rdwr_mbr->m_class == ACLUTIL_CXTYPE_STRUCT_MBR) {
		h_ctx->m_err = ACLSPV_COMPILE_CXTP_TYINVL_STRUCT_DCR;
		return 0;
	}

	if(rdwr_mbr->m_info.m_struct_mbr.m_dcr) {
		aclutil_cxtp* g;
		if((g = aclutil_cxtp_get_dcr(
						&__get_type_uniques(h_ctx)[rdwr_mbr->m_info.m_struct_mbr.m_dcr]
						, c_decor
						, h_ctx)))
			return g;
	}

	{
		struct __ret ID;
		aclutil_cxtp TAR;
		ae2fcc_memset(&TAR, 0, sizeof(TAR));

		TAR.m_class	= ACLUTIL_CXTYPE_DCR;
		TAR.m_info.m_dcr.m_op		= (aclspv_wrd_t)c_decor;
		TAR.m_info.m_dcr.m_nxt		= 0;

		ID = s_ret_new(h_ctx, &TAR, h_ctx->m_id++, 0);
		rdwr_mbr = &__get_type_uniques(h_ctx)[pad];

		unless(ID.m_id)	return 0;
		r = aclutil_find_cxtp(ID.m_id, h_ctx);
	}

	ae2f_ifezerr(r)	return 0;

	++rdwr_mbr->m_info.m_struct_mbr.m_num_dcr;
	unless(rdwr_mbr->m_info.m_struct_mbr.m_dcr) {
		rdwr_mbr->m_info.m_struct_mbr.m_dcr = (aclspv_wrd_t)(r - __get_type_uniques(h_ctx));
		return r;
	}

	return s_dcr_insert_invoke(
			(aclspv_wrd_t)c_decor
			, r
			, &(__get_type_uniques(h_ctx)[rdwr_mbr->m_info.m_struct_mbr.m_dcr])
			, h_ctx
			);
}


void aclutil_cxtp_free_struct_mbr(aclutil_cxtp* ae2f_restrict fini, h_aclspv_ctx_t h_ctx) {
	do {
		fini->m_class = ACLUTIL_CXTYPE_RESERVED;
		if(fini->m_info.m_struct_mbr.m_dcr)
			aclutil_cxtp_free_dcr(&__get_type_uniques(h_ctx)[fini->m_info.m_struct_mbr.m_dcr], h_ctx);

		fini = &__get_type_uniques(h_ctx)[fini->m_info.m_struct_mbr.m_nxt];
	} while(fini->m_class == ACLUTIL_CXTYPE_STRUCT_MBR);
}

#include <aclconst/align.h>

/** grow as struct */
#if	1
aclutil_cxtp* aclutil_cxtp_struct_add_mbr_grow(
		aclutil_cxtp* ae2f_restrict rdwr_struct,
		aclspv_id_t c_id,
		h_aclspv_ctx_t h_ctx) {
	aclutil_cxtp* ae2f_restrict mbr;
	aclspv_wrd_t pad = (aclspv_wrd_t)(__get_type_uniques(h_ctx) - rdwr_struct);

	aclspv_wrd_t new_algn = aclutil_cxtp_alignof(c_id, h_ctx);
	aclconst_specsz new_sz = aclutil_cxtp_sizeof(c_id, h_ctx);

	rdwr_struct->m_alignof = rdwr_struct->m_alignof < new_algn
		? new_algn
		: rdwr_struct->m_alignof
		;

	rdwr_struct->m_sizeof = aclconst_alignup1(rdwr_struct->m_sizeof, rdwr_struct->m_alignof, h_ctx);
	ae2f_ifezerr(mbr = aclutil_cxtp_struct_add_mbr(rdwr_struct, c_id, h_ctx))
		return 0;

	rdwr_struct = &__get_type_uniques(h_ctx)[pad];

	mbr->m_info.m_struct_mbr.m_pad = rdwr_struct->m_sizeof;

	{
		aclconst_specsz add;

#define	__ADD(r, c_x, c_y)								\
		{									\
			(r).m_id = 0;							\
			(r).m_lit.m_max = (c_x).m_lit.m_max > (c_y).m_lit.m_max;	\
		}

		_aclconst_specsize_op2(L
				, add
				, new_sz
				, rdwr_struct->m_sizeof
				, SpvOpIAdd
				, __ADD
				, h_ctx);

		rdwr_struct->m_sizeof = add;

#undef	__ADD
	}

	rdwr_struct->m_sizeof = aclconst_alignup1(rdwr_struct->m_sizeof, rdwr_struct->m_alignof, h_ctx);
	return mbr;
}
#endif

/** non-grow (union) */
aclutil_cxtp* aclutil_cxtp_struct_add_mbr_un(
		aclutil_cxtp* ae2f_restrict rdwr_struct,
		aclspv_id_t c_id,
		h_aclspv_ctx_t h_ctx) {
	aclspv_wrd_t new_algn = aclutil_cxtp_alignof(c_id, h_ctx);
	aclconst_specsz new_sz = (aclutil_cxtp_sizeof(c_id, h_ctx));

	rdwr_struct->m_alignof = rdwr_struct->m_alignof < new_algn ? new_algn : rdwr_struct->m_alignof;

	{
		aclconst_specsz cnd, gt;

#define	__GT(r, c_x, c_y)								\
		{									\
			(r).m_id = 0;							\
			(r).m_lit.m_max = (c_x).m_lit.m_max > (c_y).m_lit.m_max;	\
		}

		_aclconst_specsize_op2_bool(L
				, cnd
				, new_sz
				, rdwr_struct->m_sizeof
				, SpvOpUGreaterThan
				, __GT
				, h_ctx);

		_aclconst_specsize_op3_sel(L
				, gt
				, cnd
				, new_sz
				, rdwr_struct->m_sizeof
				, h_ctx
				);

		rdwr_struct->m_sizeof = gt;

#undef	__GT
	}

	rdwr_struct->m_sizeof = aclconst_alignup1(rdwr_struct->m_sizeof, rdwr_struct->m_alignof, h_ctx);

	{
		typedef aclutil_cxtp* ae2f_restrict pcxtp_t;
		pcxtp_t mbr, mbr_offset;

		ae2f_ifezerr(mbr = aclutil_cxtp_struct_add_mbr(rdwr_struct, c_id, h_ctx))
			return 0;

#define	rdwr_struct

		ae2f_ifezerr(mbr_offset = aclutil_cxtp_struct_mbr_dcr_cfg(mbr, SpvDecorationOffset, h_ctx))
			return 0;

		mbr->m_info.m_struct_mbr.m_pad.m_id = 0;
		mbr->m_info.m_struct_mbr.m_pad.m_lit.m_max = 0;

		mbr_offset->m_info.m_dcr.m_num_oprnd = 1;
		mbr_offset->m_info.m_dcr.m_oprnd[0] = 0x0;

#undef	rdwr_struct

		return mbr;
	}
}
