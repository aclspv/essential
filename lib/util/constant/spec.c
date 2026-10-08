#include <util/constant.h>

aclspv_id_t	aclutil_mk_constant_spec_id(
		const aclspv_wrd_t c_key, const aclspv_wrd_t c_val, 
		h_aclspv_ctx_t h_ctx
		)
{
	return 
		aclval_cfg_spec(
				c_val
				, h_ctx
				, ACLVAL_NBYTE_4 
				, ACLVAL_TY_SIGNED
				, c_key
				).m_val;
}

aclspv_id_t	aclutil_mk_constant_spec64_id(
		const aclspv_wrd_t c_key, 
		const u64_least c_val, h_aclspv_ctx_t h_ctx) {
	return 
		aclval_cfg_spec(
				c_val
				, h_ctx
				, ACLVAL_NBYTE_8
				, ACLVAL_TY_SIGNED
				, c_key
				).m_val;
}

aclspv_id_t	aclutil_mk_constant_spec_id_f32(const aclspv_wrd_t c_key, const float c_val, h_aclspv_ctx_t h_ctx)
{
	union { u32_least m_u32; float m_f32; } U;
	U.m_f32 = c_val;
	return
		aclval_cfg_spec(
				U.m_u32
				, h_ctx
				,ACLVAL_NBYTE_4 
				, ACLVAL_TY_FLT
				, c_key
				).m_val;
}

aclspv_id_t	aclutil_mk_constant_spec_id_f64(
		const aclspv_wrd_t c_key, 
		const double c_val, h_aclspv_ctx_t h_ctx) {
	union { u64_least m_u32; double m_f32; } U;
	U.m_f32 = c_val;
	return
		aclval_cfg_spec(
				U.m_u32
				, h_ctx
				, ACLVAL_NBYTE_8
				, ACLVAL_TY_FLT
				, c_key
				).m_val;
}
