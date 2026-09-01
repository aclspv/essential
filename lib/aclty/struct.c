#include "./struct.h"
#include "./implutil.h"

#include <aclmem.h>

/*** STRUCT CONFIGURATION ***/
aclspv_id_t aclty_cfg_struct(h_aclspv_ctx_t h_ctx) {
	aclutil_cxtp TAR;
	aclmemzero(&TAR, sizeof(TAR));

	TAR.m_class	= ACLUTIL_CXTYPE_STRUCT;

	{
		const struct __ret ID = s_ret_new(h_ctx, &TAR, h_ctx->m_id++, 1);
		ae2f_expected_but_else(ID.m_id) return 0;
		return	ID.m_id;
	}
}

#include "./dcr.h"

/** [WIP] */
static ae2f_ccpure aclspv_bool_t	s_is_struct_mbr_same(
		const aclutil_cxtp* ae2f_restrict rd0, 
		const aclutil_cxtp* ae2f_restrict rd1,
		h_aclspv_ctx_t h_ctx
		) {
	if(rd0->m_info.m_struct_mbr.m_id_mbr != rd1->m_info.m_struct_mbr.m_id_mbr)
		return 0;

	unless(aclconst_is_specsz_same(
				rd0->m_info.m_struct_mbr.m_pad
				, rd1->m_info.m_struct_mbr.m_pad
				, h_ctx
				)) return 0;

#if	0
	if(rd0->m_alignof != rd1->m_alignof)
		return 0;
#endif

	if(!!rd0->m_info.m_struct_mbr.m_dcr != !!rd1->m_info.m_struct_mbr.m_dcr)
		return 0;

	if(rd0->m_info.m_struct_mbr.m_dcr && !aclty_is_dcr_same(
			&__get_type_uniques(h_ctx)[rd0->m_info.m_struct_mbr.m_dcr],
			&__get_type_uniques(h_ctx)[rd1->m_info.m_struct_mbr.m_dcr],
			h_ctx)) return 0;

	if(!rd0->m_info.m_struct_mbr.m_nxt != !rd1->m_info.m_struct_mbr.m_nxt)
		return 0;

	if(!rd0->m_info.m_struct_mbr.m_nxt)
		return 1;

	rd0 = &__get_type_uniques(h_ctx)[rd0->m_info.m_struct_mbr.m_nxt];
	rd1 = &__get_type_uniques(h_ctx)[rd1->m_info.m_struct_mbr.m_nxt];

	return s_is_struct_mbr_same(rd0, rd1, h_ctx);
}

static ae2f_ccpure aclspv_bool_t	s_is_struct_same(
		const aclutil_cxtp* ae2f_restrict rd0, 
		const aclutil_cxtp* ae2f_restrict rd1,
		h_aclspv_ctx_t h_ctx) {
	if(rd0 == rd1) return 0;
	if(rd0->m_class != rd1->m_class) return 0;
	if(rd0->m_alignof != rd1->m_alignof) return 0;

#if	0
	unless(aclconst_is_specsz_same(rd0->m_sizeof, rd1->m_sizeof, h_ctx))
		return 0;
#endif

	if(rd0->m_info.m_struct.m_num_dcr != rd1->m_info.m_struct.m_num_dcr)
		return 0;

	if(rd0->m_info.m_struct.m_dcr && !aclty_is_dcr_same(
			&__get_type_uniques(h_ctx)[rd0->m_info.m_struct.m_dcr],
			&__get_type_uniques(h_ctx)[rd1->m_info.m_struct.m_dcr],
			h_ctx)) return 0;

	if(rd0->m_info.m_struct.m_num_mbr != rd1->m_info.m_struct.m_num_mbr)
		return 0;

	if(!rd0->m_info.m_struct.m_mbr)
		return 1;

	return s_is_struct_mbr_same(
			&__get_type_uniques(h_ctx)[rd0->m_info.m_struct.m_mbr]
			, &__get_type_uniques(h_ctx)[rd1->m_info.m_struct.m_mbr]
			, h_ctx
			);
}

#include "./struct/mbr.h"

static void s_free_struct(aclutil_cxtp* fini, h_aclspv_ctx_t h_ctx) {
	fini->m_class = ACLUTIL_CXTYPE_RESERVED;

	if(fini->m_info.m_struct.m_dcr)
		aclty_free_dcr(
				&__get_type_uniques(h_ctx)[fini->m_info.m_struct.m_dcr]
				, h_ctx
				);

	if(fini->m_info.m_struct.m_mbr)
		aclty_free_struct_mbr(
				&__get_type_uniques(h_ctx)[fini->m_info.m_struct.m_mbr]
				, h_ctx
				);
}

#include <util/wrdemit.h>

aclutil_cxtp* aclty_build_struct(aclspv_id_t c_id, h_aclspv_ctx_t h_ctx) {
	aclutil_cxtp* cxtp = aclty_find(c_id, h_ctx);
	aclspv_wrd_t	i = h_ctx->m_num_type_uniques;

	ae2f_ifezerr(cxtp) return 0;

	if(cxtp->m_class != ACLUTIL_CXTYPE_STRUCT)
		return 0;

	while(i --> 0) {
		aclutil_cxtp* cxtp2 = &__get_type_uniques(h_ctx)[i];
		if(s_is_struct_same(cxtp, cxtp2, h_ctx)) {
			s_free_struct(cxtp, h_ctx);
			return cxtp2;
		}
	}

	/*** make an actual type, */
	{
		aclspv_wrd_t	idx = cxtp->m_info.m_struct.m_num_mbr,
				pad = h_ctx->m_count.m_types;

		ae2f_ifezerr(h_ctx->m_count.m_types = aclutil_emit_wrd(
					&h_ctx->m_section.m_types
					, h_ctx->m_count.m_types + cxtp->m_info.m_struct.m_num_mbr + 1
					, 0
					)) {
			return 0;
		}

		while(idx-->0) {
			aclutil_cxtp* mbr = aclty_get_struct_mbr(cxtp, idx, h_ctx);
			ae2f_ifezerr(mbr) return 0;

			aclutil_get_wrd_of_vec(&h_ctx->m_section.m_types)[pad + idx + 2]
				= mbr->m_info.m_struct_mbr.m_id_mbr;


		}

		aclutil_get_wrd_of_vec(&h_ctx->m_section.m_types)[pad + 0] = aclspv_opcode_to_wrd(SpvOpTypeStruct, cxtp->m_info.m_struct.m_num_mbr + 1);
		aclutil_get_wrd_of_vec(&h_ctx->m_section.m_types)[pad + 1] = cxtp->m_type_id;
	}

	ae2f_ifnezerr_strict(aclty_build_dcr(
			&__get_type_uniques(h_ctx)[cxtp->m_info.m_struct.m_dcr]
			, cxtp->m_type_id
			, h_ctx)) return 0;

	{
		const aclutil_cxtp* ae2f_restrict mbr = &__get_type_uniques(h_ctx)[cxtp->m_info.m_struct.m_mbr];

		while(mbr->m_class == ACLUTIL_CXTYPE_STRUCT_MBR) {
			ae2f_ifnezerr_strict(aclty_build_dcr_mdcr(
						&__get_type_uniques(h_ctx)[mbr->m_info.m_struct_mbr.m_dcr]
						, cxtp->m_type_id
						, mbr->m_info.m_struct_mbr.m_idx
						, h_ctx
						)) return 0;

			mbr = &__get_type_uniques(h_ctx)[mbr->m_info.m_struct_mbr.m_nxt];
		}
	}

	return cxtp;
}

#include "./struct/mbr.h"

aclspv_id_t aclty_cfg_struct2(h_aclspv_ctx_t h_ctx, aclspv_id_t c) {
	typedef aclutil_cxtp* ae2f_restrict p_cxtp_t;

	const aclspv_id_t	NEWID = aclty_cfg_struct(h_ctx);
	p_cxtp_t ORIG = aclty_find(c, h_ctx), NEW;

	ae2f_ifezerr(ORIG)	return 0;
	ae2f_ifezerr(NEWID)	return 0;
	ae2f_ifezerr(c)		return NEWID;
	ae2f_ifnezerr_strict(ORIG->m_class != ACLUTIL_CXTYPE_STRUCT)
		return 0;

	NEW = aclty_find(NEWID, h_ctx);

	ae2f_assume_strict(NEW);
	ae2f_assume_strict(NEW->m_class == ACLUTIL_CXTYPE_STRUCT);

	/** decorator */
	if(ORIG->m_info.m_struct.m_dcr) {
		aclspv_wrd_t	IDX_DCR = ORIG->m_info.m_struct.m_dcr;
		while(IDX_DCR) {
			p_cxtp_t
				DCR = &__get_type_uniques(h_ctx)[IDX_DCR];
			p_cxtp_t
				DCR_DST = aclty_cfg_struct_dcr1(
					NEWID
					, DCR->m_info.m_dcr.m_op
					, h_ctx
					);

			ae2f_ifezerr(DCR_DST)
				return 0;

			DCR = &__get_type_uniques(h_ctx)[IDX_DCR];

			_aclutil_vcopr(L0, =
					, DCR_DST->m_info.m_dcr.m_oprnd
					, DCR->m_info.m_dcr.m_oprnd
					, MAX_ACLCXTP_DCR_OPRND);

			IDX_DCR = DCR->m_info.m_dcr.m_nxt;
		}

		ORIG	= aclty_find(c, h_ctx);
		NEW	= aclty_find(NEWID, h_ctx);
	}

	/** member */
	if(ORIG->m_info.m_struct.m_mbr) {
		aclspv_wrd_t	IDX_MBR = ORIG->m_info.m_struct.m_mbr;
		while(IDX_MBR) {
			p_cxtp_t MBR, MBR_DST;

			MBR = &__get_type_uniques(h_ctx)[IDX_MBR];

			ae2f_ifezerr(MBR_DST = aclutil_cxtp_struct_add_mbr(
						NEW
						, MBR->m_info.m_struct_mbr.m_id_mbr
						, h_ctx)) return 0;

			MBR = &__get_type_uniques(h_ctx)[IDX_MBR];
			MBR_DST->m_info.m_struct_mbr.m_pad = MBR->m_info.m_struct_mbr.m_pad;

			/** member decorator */
			if(MBR->m_info.m_struct_mbr.m_dcr) {
				const aclspv_id_t	MBRID = MBR_DST->m_type_id;
				aclspv_wrd_t	IDX_DCR = MBR->m_info.m_struct_mbr.m_dcr;

				while(IDX_DCR) {
					p_cxtp_t
						DCR = &__get_type_uniques(h_ctx)[IDX_DCR];
					p_cxtp_t
						DCR_DST = aclty_cfg_struct_mdcr1(
								MBRID
								, DCR->m_info.m_dcr.m_op
								, h_ctx
								);

					ae2f_ifezerr(DCR_DST)
						return 0;

					DCR = &__get_type_uniques(h_ctx)[IDX_DCR];

					_aclutil_vcopr(L1, =
							, DCR_DST->m_info.m_dcr.m_oprnd
							, DCR->m_info.m_dcr.m_oprnd
							, MAX_ACLCXTP_DCR_OPRND);

					IDX_DCR = DCR->m_info.m_dcr.m_nxt;
				}

				MBR = &__get_type_uniques(h_ctx)[IDX_MBR];
				NEW	= aclty_find(NEWID, h_ctx);
			}

			IDX_MBR = MBR->m_info.m_struct_mbr.m_nxt;
		}

		ORIG	= aclty_find(c, h_ctx);
	}

	return NEWID;
}
