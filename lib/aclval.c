#include <aclval.h>
#include <aclutil/emitx.h>
#include <acltrl/f16.h>
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

			const aclspv_bool_t cond = KEY1 < c_key_1 || (KEY1 == c_key_1 && KEY0 < c_key_0);

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
#include <aclty/default.h>

aclspv_id_t aclval_cfg_bool(
		const aclspv_bool_t c_val,
		h_aclspv_ctx_t h_ctx
		) {
	aclval_t* ae2f_restrict const C = aclval_cfg_node(c_val, h_ctx);
	ae2f_ifezerr(C) return 0;
	ae2f_ifezerr(aclty_mk_default(ACLSPV_ID_BOOL, h_ctx)) return 0;
	if(C->m_const_ids[ACLSPV_ID_BOOL])
		return C->m_const_ids[ACLSPV_ID_BOOL];

	{
		ae2f_ifezerr(h_ctx->m_count.m_types = aclutil_emitx_3(
				&h_ctx->m_section.m_types
				, h_ctx->m_count.m_types
				, ae2f_bll_sel_bool2(aclspv_bool_t, c_val, SpvOpConstantTrue, SpvOpConstantFalse)
				, ACLSPV_ID_BOOL
				, h_ctx->m_id)) return 0;
	}

	return C->m_const_ids[ACLSPV_ID_BOOL] = aclctx_newid(h_ctx);
}

#include <aclval/nil.h>
#include <aclty/sclr.h>

aclval_spec_t ae2f_ccpure aclval_get_spec(
		h_aclspv_ctx_t		h_ctx,
		aclspv_wrd_t		c_idx_specconst
		) {
	aclval_spec_t	DUMMY = {0, 0};
	aclval_t* ae2f_restrict const C = aclval_cfg_node(c_idx_specconst, h_ctx);
	return ae2f_expected(C) ? C->m_spec_constant : DUMMY;
}

aclspv_id_t	aclval_cfg_f16_bits(
		const aclspv_wrd_t c_bits,
		h_aclspv_ctx_t h_ctx
		) {
	return aclval_cfg(
			(aclspv_wrd2_t)(c_bits & 0xFFFFu)
			, h_ctx, ACLVAL_NBYTE_2
			, ACLVAL_TY_FLT
			);
}

#if	__ACLSPV_SCLR_BF16
aclspv_id_t	aclval_cfg_bf16_bits(
		const aclspv_wrd_t c_bits,
		h_aclspv_ctx_t h_ctx
		) {
	return aclval_cfg_by_enum(
			(aclspv_wrd2_t)(c_bits & 0xFFFFu)
			, h_ctx
			, ACLSPV_ID_BF16
			);
}
#endif

#if	__ACLSPV_SCLR_F8_E4M3
aclspv_id_t	aclval_cfg_f8_e4m3_bits(
		const aclspv_wrd_t c_bits,
		h_aclspv_ctx_t h_ctx
		) {
	return aclval_cfg_by_enum(
			(aclspv_wrd2_t)(c_bits & 0xFFu)
			, h_ctx
			, ACLSPV_ID_F8_E4M3
			);
}
#endif

#if	__ACLSPV_SCLR_F8_E5M2
aclspv_id_t	aclval_cfg_f8_e5m2_bits(
		const aclspv_wrd_t c_bits,
		h_aclspv_ctx_t h_ctx
		) {
	return aclval_cfg_by_enum(
			(aclspv_wrd2_t)(c_bits & 0xFFu)
			, h_ctx
			, ACLSPV_ID_F8_E5M2
			);
}
#endif

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

aclspv_vid_t	aclval_cfg_by_enum(const aclspv_wrd2_t c_val, h_aclspv_ctx_t h_ctx, enum ACLSPV_ID_ c_ty) {
	aclval_t* ae2f_restrict C;

	unless(c_val) {
		return aclval_cfg_nil((aclty_id_t)c_ty, h_ctx);
	}

	if(c_ty == ACLSPV_ID_BOOL) {
		return aclval_cfg_bool(!!c_val, h_ctx);
	}

	C = aclval_cfg_node(c_val, h_ctx);

	ae2f_ifezerr(C)	return 0;
	ae2f_ifezerr(aclty_mk_default((aclty_default_t) c_ty, h_ctx))
		return 0;

	if(C->m_const_ids[c_ty])
		return C->m_const_ids[c_ty];

	switch(aclty_get_sclr_width((aclty_id_t)c_ty) >> 5u) {
		case 2:
			ae2f_ifezerr(h_ctx->m_count.m_types = aclutil_emitx_5(
						&h_ctx->m_section.m_types
						, h_ctx->m_count.m_types
						, SpvOpConstant
						, (aclty_id_t)c_ty
						, h_ctx->m_id
						, (aclspv_wrd_t)(c_val & 0xFFFFFFFF)
						, (aclspv_wrd_t)((c_val >> 32) & 0xFFFFFFFF)
						)) return 0;
			break;

		case 0:
		case 1:
			ae2f_ifezerr(h_ctx->m_count.m_types = aclutil_emitx_4(
						&h_ctx->m_section.m_types
						, h_ctx->m_count.m_types
						, SpvOpConstant
						, (aclty_id_t)c_ty
						, h_ctx->m_id
						, (aclspv_wrd_t)(c_val & 0xFFFFFFFF)
						)) return 0;
			break;
		default:
			return 0;
	}

	return C->m_const_ids[c_ty] = aclctx_newid(h_ctx);
}

aclval_spec_t	aclval_cfg_spec_by_enum(
		const aclspv_wrd_t c_key,
		const aclspv_wrd2_t c_val,
		h_aclspv_ctx_t h_ctx,
		enum ACLSPV_ID_ c_ty
		) {
	const aclval_spec_t	ZERO = {0, 0};
	aclval_t* ae2f_restrict const C = aclval_cfg_node(c_key, h_ctx);

	ae2f_expected_but_else(C)	return ZERO;
	ae2f_expected_but_else(aclty_mk_default((aclty_default_t)c_ty, h_ctx))
		return ZERO;

	if(C->m_spec_constant.m_val)
		return C->m_spec_constant.m_ty == (aclty_id_t)c_ty ? C->m_spec_constant : ZERO;	

	switch(aclty_get_sclr_width((aclty_id_t)c_ty) >> 5u) {
		case 2:
			ae2f_ifezerr(h_ctx->m_count.m_types = aclutil_emitx_5(
						&h_ctx->m_section.m_types
						, h_ctx->m_count.m_types
						, SpvOpSpecConstant
						, (aclspv_wrd_t)c_ty 
						, h_ctx->m_id
						, (aclspv_wrd_t)(c_val & 0xFFFFFFFF)
						, (aclspv_wrd_t)((c_val >> 32) & 0xFFFFFFFF)
						)) return ZERO;
			break;

		case 0:
		case 1:
			ae2f_ifezerr(h_ctx->m_count.m_types = aclutil_emitx_4(
						&h_ctx->m_section.m_types
						, h_ctx->m_count.m_types
						, SpvOpSpecConstant
						, (aclspv_wrd_t)c_ty 
						, h_ctx->m_id
						, (aclspv_wrd_t)(c_val & 0xFFFFFFFF)
						)) return ZERO;
			break;
		default:
			ae2fpat_assertassume(0);
			return ZERO;
	}

	C->m_spec_constant.m_ty		= (aclspv_id_t)c_ty;
	C->m_spec_constant.m_val	= aclctx_newid(h_ctx);

	return C->m_spec_constant;
}
