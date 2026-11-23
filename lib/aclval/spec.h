#ifndef	aclval_spec_h
#define aclval_spec_h

#include <aclval.h>

ae2f_inline_f static aclspv_vid_t	aclval_cfg_spec_i32(
		const aclspv_wrd_t c_key, const aclspv_wrd_t c_val, 
		h_aclspv_ctx_t h_ctx
		)
{
	return 
		aclval_cfg_spec_by_enum(
				c_key
				, c_val
				, h_ctx
				, ACLSPV_ID_I32
				).m_val;
}

ae2f_inline_f static aclspv_vid_t	aclval_cfg_spec_i64(
		const aclspv_wrd_t c_key, 
		const aclspv_wrd2_t c_val, h_aclspv_ctx_t h_ctx) {
	return 
		aclval_cfg_spec_by_enum(
				c_key
				, c_val
				, h_ctx
				, ACLSPV_ID_I64
				).m_val;
}

ae2f_inline_f static aclspv_vid_t	aclval_cfg_spec_f32(const aclspv_wrd_t c_key, const float c_val, h_aclspv_ctx_t h_ctx)
{
	union { aclspv_wrd_t m_u32; float m_f32; } U;
	U.m_f32 = c_val;
	return
		aclval_cfg_spec_by_enum(
				c_key
				, U.m_u32
				, h_ctx
				, ACLSPV_ID_F32
				).m_val;
}

ae2f_inline_f static aclspv_vid_t	aclval_cfg_spec_f64(
		const aclspv_wrd_t c_key, 
		const double c_val, h_aclspv_ctx_t h_ctx) {
	union { u64_least m_u32; double m_f32; } U;
	U.m_f32 = c_val;
	return
		aclval_cfg_spec_by_enum(
				c_key
				, U.m_u32
				, h_ctx
				, ACLSPV_ID_F64
				).m_val;
}

#endif
