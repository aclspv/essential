#if	__ACLSPV_MATH_NR

#include <aclty.h>
#include "./rsqrtf_nr.h"
#include <aclexpr/carve/op2/arithmetic.h>
#include <aclexpr/carve/op2/shift.h>

typedef float f32;
typedef aclspv_wrd_t u32;

typedef f32 f32_check[sizeof(f32) == 4];
typedef u32 u32_check[sizeof(u32) == 4];

#include <util/literal.h>

typedef union { f32 m_f; u32 m_u; } f32_u;

static aclutil_literal rsqrtf_nr(aclutil_literal c) {
	const f32 chalf = c.m_flt * 0.5f;
	int i;

	c.m_wrd = __ACLSPV_MATH_NR_RSQRTF_MAGIC - (c.m_wrd >> 1);

	for(i = __ACLSPV_MATH_NR_IT; i --> 0; ) {
		const f32 y2 = c.m_flt * c.m_flt;
		c.m_flt = c.m_flt * (1.5f - (chalf * y2));
	}
	
	return c;
}

	aclspv_bool_t aclexpr_carve_op1_rsqrtf_nr(
			struct aclexpr_visit_data	ae2fcc_arg_sarr_restrict(rdwr_data, 1),
			aclspv_id_t			ae2fcc_arg_sarr_restrict(r_prv_idval, 1),
			aclexpr_opbincfg_extra		ae2fcc_arg_sarr_restrict(r_prv_extra, 1),
			aclspv_wrd_t			ae2fcc_arg_sarr_restrict(rdwr_lst_scale_buf, ACLEXPR_OPBINCFG_END + 1),
			aclexpr_opbincfg_extra		ae2fcc_arg_sarr_restrict(rdwr_lst_extra, 1),
			aclutil_literal			ae2fcc_arg_sarr_restrict(r_ltrl, 1)
			)
{
	aclexpr_opbincfg_extra x = {0,};
	aclexpr_opbincfg_extra x_half = {0,};
	aclexpr_opbincfg_extra x_u32 = {0,};
	aclexpr_opbincfg_extra x_u32_half = {0,};
	aclexpr_opbincfg_extra magic = {0,};
	aclexpr_opbincfg_extra one = {0, };
	aclexpr_opbincfg_extra half = {0, };
	aclexpr_opbincfg_extra nr_startup = {0, };

	aclexpr_opbincfg_extra buf_extra[2];

	aclspv_id_t f32_like_ty, u32_like_ty;
	aclspv_id_t x_id;
	aclspv_id_t x_u32_id;
	aclspv_id_t x_u32_half_id;
	aclspv_id_t magic_id;
	aclspv_id_t one_id;
	aclspv_id_t half_id;
	aclspv_id_t nr_startup_id;

	aclspv_wrd_t buf[ACLEXPR_OPBINCFG_END + 2];

	magic.m_literal.m_wrd = __ACLSPV_MATH_NR_RSQRTF_MAGIC;
	magic.m_mask_literal = ACLUTIL_LITERAL_MASK_LITERAL;
	magic.m_type_original = ACLSPV_ID_U32;
	magic_id = aclval_cfg_by_enum(__ACLSPV_MATH_NR_RSQRTF_MAGIC, rdwr_data->m_ctx, ACLSPV_ID_U32);

	one.m_literal.m_wrd = 1;
	one.m_mask_literal = ACLUTIL_LITERAL_MASK_LITERAL;
	one.m_type_original = ACLSPV_ID_U32;
	one_id = aclval_cfg_by_enum(1, rdwr_data->m_ctx, ACLSPV_ID_U32);

	half.m_literal.m_flt = 0.5f;
	half.m_mask_literal = ACLUTIL_LITERAL_MASK_LITERAL;
	half.m_type_original = ACLSPV_ID_F32;
	half_id = aclval_cfg_f32(0.5f, rdwr_data->m_ctx);

	r_prv_extra->m_mask_literal = (aclspv_wrd_t)ACLUTIL_LITERAL_LITERAL;

	ae2f_ifezerr(aclty_is_simd(rdwr_lst_extra[0].m_type_original, rdwr_data->m_ctx)) {
		return ACLEXPR_CARVE_NO;
	}

	ae2f_ifezerr(f32_like_ty = aclty_simd_chxg_cls(
				rdwr_lst_extra[0].m_type_original
				, aclid_mk_default_id(ACLSPV_ID_F32, rdwr_data->m_ctx)
				, rdwr_data->m_ctx
				)) return ACLEXPR_CARVE_NO;

	ae2f_ifezerr(u32_like_ty = aclty_simd_chxg_cls(
				rdwr_lst_extra[0].m_type_original
				, aclid_mk_default_id(ACLSPV_ID_U32, rdwr_data->m_ctx)
				, rdwr_data->m_ctx
				)) return ACLEXPR_CARVE_NO;

	switch(aclexpr_arithmetic_cast(
				rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_END]
				, rdwr_lst_extra->m_type_original
				, rdwr_lst_extra->m_mask_literal
				, rdwr_lst_extra->m_literal
				, &x_id
				, f32_like_ty
				, &x.m_literal
				, rdwr_data->m_ctx)) {
		default:
			ae2f_unreachable();

		case ACLEXPR_RET_NOT_THE_CASE:
		case ACLEXPR_RET_FAILURE:
			return ACLEXPR_CARVE_NO;

		case ACLEXPR_RET_SUCCESS:
		case ACLEXPR_RET_SUCCESS_CONSTANT:
		case ACLEXPR_RET_SUCCESS_LITERAL:
			break;
	}

	switch(aclexpr_arithmetic_cast(
				rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_END]
				, rdwr_lst_extra->m_type_original
				, rdwr_lst_extra->m_mask_literal
				, rdwr_lst_extra->m_literal
				, &x_u32_id
				, u32_like_ty
				, &x_u32.m_literal
				, rdwr_data->m_ctx)) {
		default:
			ae2f_unreachable();

		case ACLEXPR_RET_NOT_THE_CASE:
		case ACLEXPR_RET_FAILURE:
			return ACLEXPR_CARVE_NO;

		case ACLEXPR_RET_SUCCESS_LITERAL:
			*r_ltrl = rsqrtf_nr(x.m_literal);
			*r_prv_idval = aclval_cfg_f32(r_ltrl->m_flt, rdwr_data->m_ctx);

			{
				*r_prv_extra = one;
				r_prv_extra->m_type_original = ACLSPV_ID_F32;
			}

			return ACLEXPR_CARVE_OK;

		case ACLEXPR_RET_SUCCESS:
		case ACLEXPR_RET_SUCCESS_CONSTANT:
			break;
	}

	buf[ACLEXPR_OPBINCFG_SPECCONSTANTOP]	= aclspv_opcode_to_wrd(SpvOpSpecConstantOp, ACLEXPR_OPBINCFG_END + 1);
	buf[ACLEXPR_OPBINCFG_RESTYID]		= u32_like_ty;
	buf[ACLEXPR_OPBINCFG_OPCODE]		= ACLEXPR_OP2_SHR;
	buf[ACLEXPR_OPBINCFG_NUMOPRND]		= 2;
	buf[ACLEXPR_OPBINCFG_END]		= x_u32_id;
	buf[ACLEXPR_OPBINCFG_END + 1]		= one_id;

	buf_extra[0] = x_u32;
	buf_extra[1] = one;

	ae2f_ifnezerr_strict(aclexpr_carve2_shift(
				rdwr_data
				, &x_u32_half_id
				, &x_u32_half
				, &buf[0]
				, &buf_extra[0]))
		return ACLEXPR_CARVE_NO;

	buf[ACLEXPR_OPBINCFG_OPCODE]	= ACLEXPR_OP2_SUB;
	buf[ACLEXPR_OPBINCFG_END]	= magic_id;
	buf[ACLEXPR_OPBINCFG_END + 1]	= x_u32_half_id;

	buf_extra[0] = magic;
	buf_extra[1] = x_u32_half;

	ae2f_ifnezerr_strict(aclexpr_carve2_arithmetic(
				rdwr_data
				, &nr_startup_id
				, &nr_startup
				, &buf[0]
				, &buf_extra[0]
				, &nr_startup.m_literal))
		return ACLEXPR_CARVE_NO;
}
#else
typedef int dummy_t;
#endif
