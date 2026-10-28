
#include <aclval.h>
#include <util/emitx.h>
#include <util/iddef.h>
#include <ae2f/c90/stdbool.h>

#include <ae2f/pat/dbg.h>
#include <aclmem.h>

aclval_t*  aclval_cfg_node2(
		const aclspv_wrd_t	c_key_0,
		const aclspv_wrd_t	c_key_1,
		h_aclspv_ctx_t		h_ctx
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
					, sizeof(aclval_t)
					, ACLSPV_COMPILE_ALLOC_FAILED_CONSTANT
					)) return 0;

#define	LEFT	LR[1]
#define	RIGHT	LR[0]

	LEFT	= 0;
	RIGHT	= COUNT;

	if(COUNT && h_ctx->m_constant_cache.m_p) {
		while(LEFT < RIGHT) {
			const aclspv_wrdcount_t	MIDDLE	= LEFT + ((RIGHT - LEFT) >> 1);
			const aclspv_wrd_t	KEY0 = (((p_aclval_t)h_ctx->m_constant_cache.m_p)
					[MIDDLE].m_key_0);
			const aclspv_wrd_t	KEY1 = (((p_aclval_t)h_ctx->m_constant_cache.m_p)
					[MIDDLE].m_key_1);

			const bool cond = KEY1 < c_key_1 || (KEY1 == c_key_1 && KEY0 < c_key_0);

			if(c_key_0 == KEY0 && c_key_1 == KEY1)
				return &((p_aclval_t)h_ctx->m_constant_cache.m_p)[MIDDLE];

			/** is `KEY_MAKE < c_key ? (LEFT = MIDDLE + 1) : (RIGHT = MIDDLE)` */
			LR[cond] = ((cond) ? MIDDLE + 1 : MIDDLE);
		}
	}

	ae2f_ifnezerr_strict(s_aclmem_cgrow2(
				&h_ctx->m_constant_cache
				, COUNT + 1
				, sizeof(aclval_t)
				, ACLSPV_COMPILE_ALLOC_FAILED_CONSTANT))
		return 0;


	if(LEFT < COUNT) {
		aclspv_wrd_t		C = COUNT - LEFT;
		p_aclval_t	BASE = &((p_aclval_t)(h_ctx->m_constant_cache.m_p))[COUNT];

		while( C --> 0 ) {
			BASE[0] = BASE[-1];
			(void)--BASE;
		}
	} 

	/** sanity check for future possibility of having more elements */
	aclmemzero(&((p_aclval_t)(h_ctx->m_constant_cache.m_p))[LEFT], sizeof(aclval_t));

	((p_aclval_t)(h_ctx->m_constant_cache.m_p))[LEFT].m_key_0
		= c_key_0;
	((p_aclval_t)(h_ctx->m_constant_cache.m_p))[LEFT].m_key_1
		= c_key_1;
	((p_aclval_t)(h_ctx->m_constant_cache.m_p))[LEFT].m_set_bind_storage_class
		= (aclspv_wrd_t)SpvStorageClassMax;

	++h_ctx->m_num_constant_cache;

	return &((p_aclval_t)(h_ctx->m_constant_cache.m_p))[LEFT];

#undef	LEFT
#undef	RIGHT
}

#include <ae2f/bll.h>

aclspv_id_t aclval_cfg_bool(
		const aclspv_bool_t c_val,
		h_aclspv_ctx_t h_ctx
		) {
	aclval_t* ae2f_restrict const C = aclval_cfg_node((u64_least)c_val, h_ctx);
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

#include <aclval/nil.h>

aclspv_id_t aclval_cfg(
		const aclspv_wrd2_t	c_val,
		h_aclspv_ctx_t		h_ctx,
		enum ACLVAL_NBYTE_	c_idx_nbyte,
		enum ACLVAL_TY_	c_idx_ty
		) {
	const enum ACLSPV_ID_ MAP[ACLVAL_TY_MAX][ACLVAL_NBYTE_MAX] = {
		{ ACLSPV_ID_I8, ACLSPV_ID_I16, ACLSPV_ID_I32, ACLSPV_ID_I64 },
		{ ACLSPV_ID_U8, ACLSPV_ID_U16, ACLSPV_ID_U32, ACLSPV_ID_U64 },
		{ 0, ACLSPV_ID_F16, ACLSPV_ID_F32, ACLSPV_ID_F64 }
	};

	aclval_t* ae2f_restrict C;

	unless(c_val) {
		return aclval_cfg_nil(MAP[c_idx_ty][c_idx_nbyte], h_ctx);
	}

	if(c_idx_nbyte <= ACLVAL_NBYTE_4 && c_val >> 32) {
		return aclval_cfg(
				c_val & 0xFFFFFFFF
				, h_ctx
				, c_idx_nbyte
				, c_idx_ty);
	}

	C = aclval_cfg_node(c_val, h_ctx);

	ae2f_ifezerr(C)	return 0;
	ae2f_ifezerr(aclid_mk_default_id((B_id_default_t) MAP[c_idx_ty][c_idx_nbyte], h_ctx))
		return 0;

	if(C->m_const_ids[c_idx_ty][c_idx_nbyte]) return C->m_const_ids[c_idx_ty][c_idx_nbyte];

	if(c_idx_nbyte == ACLVAL_NBYTE_8) {
		ae2f_ifezerr(h_ctx->m_count.m_types = aclutil_emitx_5(
					&h_ctx->m_section.m_types
					, h_ctx->m_count.m_types
					, SpvOpConstant
					, (aclspv_wrd_t) MAP[c_idx_ty][c_idx_nbyte]
					, h_ctx->m_id
					, (u32_least)(c_val & 0xFFFFFFFF)
					, (u32_least)((c_val >> 32) & 0xFFFFFFFF)
					)) return 0;
	} else {
		ae2f_ifezerr(h_ctx->m_count.m_types = aclutil_emitx_4(
					&h_ctx->m_section.m_types
					, h_ctx->m_count.m_types
					, SpvOpConstant
					, (uint_least32_t) MAP[c_idx_ty][c_idx_nbyte]
					, h_ctx->m_id
					, (u32_least)(c_val & 0xFFFFFFFF)
					)) return 0;
	}

	return C->m_const_ids[c_idx_ty][c_idx_nbyte] = h_ctx->m_id++;
}

aclval_spec_t ae2f_ccpure aclval_get_spec(
		h_aclspv_ctx_t		h_ctx,
		aclspv_wrd_t		c_idx_specconst
		) {
	aclval_spec_t	DUMMY = {0, 0};
	aclval_t* ae2f_restrict const C = aclval_cfg_node(c_idx_specconst, h_ctx);
	return ae2f_expected(C) ? C->m_spec_constant : DUMMY;
}

aclval_spec_t aclval_cfg_spec(
		const u64_least		c_val,
		h_aclspv_ctx_t		h_ctx,
		enum ACLVAL_NBYTE_	c_idx_nbyte,
		enum ACLVAL_TY_	c_idx_ty,
		aclspv_wrd_t		c_idx_specconst
		) {

	const aclval_spec_t	ZERO = {0, 0};

	const enum ACLSPV_ID_ MAP[ACLVAL_TY_MAX][ACLVAL_NBYTE_MAX] = {
		{ ACLSPV_ID_I8, ACLSPV_ID_I16, ACLSPV_ID_I32, ACLSPV_ID_I64 },
		{ ACLSPV_ID_U8, ACLSPV_ID_U16, ACLSPV_ID_U32, ACLSPV_ID_U64 },
		{ 0, ACLSPV_ID_F16, ACLSPV_ID_F32, ACLSPV_ID_F64 }
	};

	aclval_t* ae2f_restrict const C = aclval_cfg_node(c_idx_specconst, h_ctx);

	ae2f_expected_but_else(C)	return ZERO;
	ae2f_expected_but_else(aclid_mk_default_id((B_id_default_t) MAP[c_idx_ty][c_idx_nbyte], h_ctx))
		return ZERO;

	if(C->m_spec_constant.m_val)
		return C->m_spec_constant.m_ty == (aclspv_id_t) MAP[c_idx_ty][c_idx_nbyte] ? C->m_spec_constant : ZERO;

	if(c_idx_nbyte == ACLVAL_NBYTE_8) {
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

aclspv_id_t	aclval_cfg_f32(const float c_val, h_aclspv_ctx_t h_ctx)
{
	union {
		float		m_flt;
		aclspv_wrd_t	m_u32;
	} FLT_U32;
	FLT_U32.m_flt = c_val;

	return aclval_cfg(
			FLT_U32.m_u32
			, h_ctx, ACLVAL_NBYTE_4
			, ACLVAL_TY_FLT
			);
}

aclspv_id_t	aclval_cfg_f64(const double c_val, h_aclspv_ctx_t h_ctx)
{
	union {
		double		m_flt;
		aclspv_wrd2_t	m_u64;
	} FLT_U64;

	FLT_U64.m_flt = c_val;

	return aclval_cfg(
			FLT_U64.m_u64
			, h_ctx, ACLVAL_NBYTE_8
			, ACLVAL_TY_FLT
			);
}
