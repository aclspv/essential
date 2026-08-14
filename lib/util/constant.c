
#include "./constant.h"
#include "./emitx.h"
#include "./iddef.h"

#include <ae2f/pat/dbg.h>
#include <ae2f/cc/memset.h>
#include <aclmem.h>

aclutil_constant*  aclutil_mk_constant_node(
		const u64_least	c_key,
		h_aclspv_ctx_t	h_ctx
		)
{
	const aclspv_wrdcount_t	COUNT = h_ctx
		? (aclspv_wrd_t)(h_ctx->m_num_constant_cache)
		: (aclspv_wrd_t)0;

	aclspv_wrdcount_t	LR[2];

	ae2fpat_assertassume_strict(h_ctx);

	if(COUNT)
	ae2f_ifnezerr_strict(s_aclmem_cgrow2(
			&h_ctx->m_constant_cache
			, COUNT
			, sizeof(aclutil_constant)
			, ACLSPV_COMPILE_ALLOC_FAILED_CONSTANT
			)) return 0;

#define	LEFT	LR[1]
#define	RIGHT	LR[0]

	LEFT	= 0;
	RIGHT	= COUNT;

	if(COUNT && h_ctx->m_constant_cache.m_p) {
#define	KEY_MADE_L \
		((u64_least)(((p_aclutil_constant_t)h_ctx->m_constant_cache.m_p)	\
			[LEFT].m_key_0)							\
			| ((u64_least)((p_aclutil_constant_t)h_ctx->m_constant_cache.m_p)	\
				[LEFT].m_key_1) << 32)

		while(LEFT < RIGHT) {
			const aclspv_wrdcount_t	MIDDLE	= LEFT + ((RIGHT - LEFT) >> 1);
			const uint_least64_t	KEY_MADE =
				(u64_least)(((p_aclutil_constant_t)h_ctx->m_constant_cache.m_p)
						[MIDDLE].m_key_0)
				| ((u64_least)((p_aclutil_constant_t)h_ctx->m_constant_cache.m_p)
						[MIDDLE].m_key_1) << 32;

			if(KEY_MADE == c_key)
				return &((p_aclutil_constant_t)h_ctx->m_constant_cache.m_p)[MIDDLE];


			/** is `KEY_MAKE < c_key ? (LEFT = MIDDLE + 1) : (RIGHT = MIDDLE)` */
			LR[KEY_MADE < c_key] = MIDDLE + (KEY_MADE < c_key);
		}

		if(KEY_MADE_L == c_key)
			return &((p_aclutil_constant_t)h_ctx->m_constant_cache.m_p)[LEFT];

#undef	KEY_MADE_L
	}

	ae2f_ifnezerr_strict(s_aclmem_cgrow2(
				&h_ctx->m_constant_cache
				, COUNT + 1
				, sizeof(aclutil_constant), ACLSPV_COMPILE_ALLOC_FAILED_CONSTANT))
		return 0;

	if(COUNT > LEFT) {
		aclspv_wrd_t		C = COUNT - LEFT;
		p_aclutil_constant_t	BASE = &((p_aclutil_constant_t)(h_ctx->m_constant_cache.m_p))[COUNT - 1];

		ae2fpat_assertassume_strict(COUNT - LEFT <= COUNT);

		while( C --> 0 ) {
			BASE[1] = BASE[0];
			(void)--BASE;
		}
	}

	/** sanity check for future possibility of having more elements */
	ae2fcc_memset(&((p_aclutil_constant_t)(h_ctx->m_constant_cache.m_p))[LEFT], 0, sizeof(aclutil_constant));
	((p_aclutil_constant_t)(h_ctx->m_constant_cache.m_p))[LEFT].m_key_0
		= (u32_least)(c_key & 0xFFFFFFFF);
	((p_aclutil_constant_t)(h_ctx->m_constant_cache.m_p))[LEFT].m_key_1
		= (u32_least)(c_key >> 32) & 0xFFFFFFFF;

	++h_ctx->m_num_constant_cache;

	return &((p_aclutil_constant_t)(h_ctx->m_constant_cache.m_p))[LEFT];

#undef	LEFT
#undef	RIGHT
}

#include <ae2f/bll.h>

aclspv_id_t aclutil_mk_constant_bool_id(
		const aclspv_bool_t c_val,
		h_aclspv_ctx_t h_ctx
		) {
	aclutil_constant* ae2f_restrict const C = aclutil_mk_constant_node((u64_least)c_val, h_ctx);
	ae2f_ifezerr(C) return 0;
	if(C->m_const_val_bool) return C->m_const_val_bool;

	ae2f_expected_but_else(aclid_mk_default_id(ACLSPV_ID_BOOL, h_ctx))
		return 0;

	ae2f_expected_but_else(h_ctx->m_count.m_types = aclutil_emitx_3(
				&h_ctx->m_section.m_types
				, h_ctx->m_count.m_types
				, ae2f_bll_sel_bool1(aclspv_wrd_t, c_val, SpvOpConstantTrue, SpvOpConstantFalse)
				, ACLSPV_ID_BOOL
				, h_ctx->m_id
				)) return 0;

	return C->m_const_val_bool = h_ctx->m_id++;
}

aclspv_id_t aclutil_mk_constant_val_id1(
		const u64_least		c_val,
		h_aclspv_ctx_t		h_ctx,
		enum ACLUTIL_CV_NBYTE_	c_idx_nbyte,
		enum ACLUTIL_CV_TY_	c_idx_ty
		) {
	const enum ACLSPV_ID_ MAP[ACLUTIL_CV_TY_MAX][ACLUTIL_CV_NBYTE_MAX] = {
		{ ACLSPV_ID_I8, ACLSPV_ID_I16, ACLSPV_ID_I32, ACLSPV_ID_I64 },
		{ ACLSPV_ID_U8, ACLSPV_ID_U16, ACLSPV_ID_U32, ACLSPV_ID_U64 },
		{ 0, ACLSPV_ID_F16, ACLSPV_ID_F32, ACLSPV_ID_F64 }
	};


#if	1
	aclutil_constant* ae2f_restrict const C = aclutil_mk_constant_node(c_val, h_ctx);
	ae2f_ifezerr(C)	return 0;
	ae2f_ifezerr(aclid_mk_default_id((B_id_default_t) MAP[c_idx_ty][c_idx_nbyte], h_ctx))
		return 0;

	if(C->m_const_ids[c_idx_ty][c_idx_nbyte]) return C->m_const_ids[c_idx_ty][c_idx_nbyte];

	if(c_idx_nbyte == ACLUTIL_CV_NBYTE_8) {
		ae2f_expected_but_else(h_ctx->m_count.m_types = aclutil_emitx_5(
					&h_ctx->m_section.m_types
					, h_ctx->m_count.m_types
					, SpvOpConstant
					, (aclspv_wrd_t) MAP[c_idx_ty][c_idx_nbyte]
					, h_ctx->m_id
					, (u32_least)(c_val & 0xFFFFFFFF)
					, (u32_least)((c_val >> 32) & 0xFFFFFFFF)
					)) return 0;
	} else {
		ae2f_expected_but_else(h_ctx->m_count.m_types = aclutil_emitx_4(
					&h_ctx->m_section.m_types
					, h_ctx->m_count.m_types
					, SpvOpConstant
					, (uint_least32_t) MAP[c_idx_ty][c_idx_nbyte]
					, h_ctx->m_id
					, (u32_least)(c_val & 0xFFFFFFFF)
					)) return 0;
	}

	return C->m_const_ids[c_idx_ty][c_idx_nbyte] = h_ctx->m_id++;
#endif
}

aclutil_specconstant_val_t ae2f_ccpure aclutil_get_specconstant_val_id(
		h_aclspv_ctx_t		h_ctx,
		aclspv_wrd_t		c_idx_specconst
		) {
	aclutil_specconstant_val_t	DUMMY = {0, 0};
	aclutil_constant* ae2f_restrict const C = aclutil_mk_constant_node(c_idx_specconst, h_ctx);
	return ae2f_expected(C) ? C->m_spec_constant : DUMMY;
}

aclutil_specconstant_val_t aclutil_mk_specconstant_val_id(
		const u64_least		c_val,
		h_aclspv_ctx_t		h_ctx,
		enum ACLUTIL_CV_NBYTE_	c_idx_nbyte,
		enum ACLUTIL_CV_TY_	c_idx_ty,
		aclspv_wrd_t		c_idx_specconst
		) {

	const aclutil_specconstant_val_t	ZERO = {0, 0};

	const enum ACLSPV_ID_ MAP[ACLUTIL_CV_TY_MAX][ACLUTIL_CV_NBYTE_MAX] = {
		{ ACLSPV_ID_I8, ACLSPV_ID_I16, ACLSPV_ID_I32, ACLSPV_ID_I64 },
		{ ACLSPV_ID_U8, ACLSPV_ID_U16, ACLSPV_ID_U32, ACLSPV_ID_U64 },
		{ 0, ACLSPV_ID_F16, ACLSPV_ID_F32, ACLSPV_ID_F64 }
	};

	aclutil_constant* ae2f_restrict const C = aclutil_mk_constant_node(c_idx_specconst, h_ctx);

	ae2f_expected_but_else(C)	return ZERO;
	ae2f_expected_but_else(aclid_mk_default_id((B_id_default_t) MAP[c_idx_ty][c_idx_nbyte], h_ctx))
		return ZERO;

	if(C->m_spec_constant.m_val)
		return C->m_spec_constant.m_ty == (aclspv_id_t) MAP[c_idx_ty][c_idx_nbyte] ? C->m_spec_constant : ZERO;

	if(c_idx_nbyte == ACLUTIL_CV_NBYTE_8) {
		ae2f_expected_but_else(h_ctx->m_count.m_types = aclutil_emitx_5(
					&h_ctx->m_section.m_types
					, h_ctx->m_count.m_types
					, SpvOpSpecConstant
					, (aclspv_wrd_t) MAP[c_idx_ty][c_idx_nbyte]
					, h_ctx->m_id
					, (u32_least)(c_val & 0xFFFFFFFF)
					, (u32_least)((c_val >> 32) & 0xFFFFFFFF)
					)) return ZERO;
	} else {
		ae2f_expected_but_else(h_ctx->m_count.m_types = aclutil_emitx_4(
					&h_ctx->m_section.m_types
					, h_ctx->m_count.m_types
					, SpvOpSpecConstant
					, (uint_least32_t) MAP[c_idx_ty][c_idx_nbyte]
					, h_ctx->m_id
					, (u32_least)(c_val & 0xFFFFFFFF)
					)) return ZERO;
	}

	C->m_spec_constant.m_ty		= (aclspv_id_t) MAP[c_idx_ty][c_idx_nbyte];
	C->m_spec_constant.m_val	= h_ctx->m_id++;

	return C->m_spec_constant;
}

aclspv_id_t	aclutil_mk_constant_val_f32_id(const float c_val, h_aclspv_ctx_t h_ctx)
{
	union {
		float		m_flt;
		u32_least	m_u32;
	} FLT_U32;
	FLT_U32.m_flt = c_val;

	return aclutil_mk_constant_val_id1(
			FLT_U32.m_u32
			, h_ctx, ACLUTIL_CV_NBYTE_4
			, ACLUTIL_CV_TY_FLT
			);
}

aclspv_id_t	aclutil_mk_constant_val_f64_id(const double c_val, h_aclspv_ctx_t h_ctx)
{
	union {
		double		m_flt;
		u64_least	m_u64;
	} FLT_U64;

	FLT_U64.m_flt = c_val;

	return aclutil_mk_constant_val_id1(
			FLT_U64.m_u64
			, h_ctx, ACLUTIL_CV_NBYTE_8
			, ACLUTIL_CV_TY_FLT
			);
}
