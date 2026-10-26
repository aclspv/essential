#if	__ACLSPV_MATH_NR

#include <aclty.h>
#include "./rsqrtf_nr.h"

#include <aclexpr/carve/util.h>
#include <aclval.h>
#include <aclcur/op.h>

#include <util/literal.h>

typedef float f32;
typedef aclspv_wrd_t u32;

typedef f32 f32_check[sizeof(f32) == 4];
typedef u32 u32_check[sizeof(u32) == 4];

typedef union { f32 m_f; u32 m_u; } f32_u;

static aclutil_literal rsqrtf_nr(aclutil_literal c) {
	f32_u u;
	f32 chalf;
	int i;
	u.m_f = c.m_flt;
	chalf = u.m_f * 0.5f;
	u.m_u = __ACLSPV_MATH_NR_RSQRTF_MAGIC - (u.m_u >> 1);

	for(i = __ACLSPV_MATH_NR_IT; i --> 0; ) {
		const f32 y2 = u.m_f * u.m_f;
		u.m_f = u.m_f * (1.5f - (chalf * y2));
	}

	c.m_flt = u.m_f;
	c.m_wrd = u.m_u;
	return c;
}

	aclspv_bool_t aclexpr_carve_op1_rsqrtf_nr(
			struct aclexpr_visit_data	ae2fcc_arg_sarr_restrict(rdwr_data, 1),
			aclspv_wrd_t			ae2fcc_arg_sarr_restrict(rdwr_prv_idval, 1),
			aclexpr_opbincfg_extra		ae2fcc_arg_sarr_restrict(rdwr_prv_extra, 1),
			aclspv_wrd_t			ae2fcc_arg_sarr_restrict(rdwr_lst_scale_buf, ACLEXPR_OPBINCFG_END + 1),
			aclexpr_opbincfg_extra		ae2fcc_arg_sarr_restrict(rdwr_lst_extra, 1),
			aclutil_literal			ae2fcc_arg_sarr_restrict(r_ltrl, 1)
			)
{
	aclspv_id_t f32_like_ty, u32_like_ty;
	aclspv_id_t x_id;
	aclspv_id_t half_id, threehalf_id, magic_id, one_id;

	h_aclspv_ctx_t ctx = rdwr_data->m_ctx;

	ae2f_ifezerr(aclty_is_simd(rdwr_lst_extra[0].m_type_original, ctx)) {
		return ACLEXPR_CARVE_NO;
	}

	ae2f_ifezerr(f32_like_ty = aclty_simd_chxg_cls(
				rdwr_lst_extra[0].m_type_original
				, aclid_mk_default_id(ACLSPV_ID_F32, ctx)
				, ctx
				)) return ACLEXPR_CARVE_NO;

	ae2f_ifezerr(u32_like_ty = aclty_simd_chxg_cls(
				rdwr_lst_extra[0].m_type_original
				, aclid_mk_default_id(ACLSPV_ID_U32, ctx)
				, ctx
				)) return ACLEXPR_CARVE_NO;

	rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_RESTYID] = f32_like_ty;

	/* scalar seeds */
	magic_id = aclval_cfg_by_enum(__ACLSPV_MATH_NR_RSQRTF_MAGIC, ctx, ACLSPV_ID_U32);
	one_id = aclval_cfg_by_enum(1, ctx, ACLSPV_ID_U32);
	half_id = aclval_cfg_f32(0.5f, ctx);
	threehalf_id = aclval_cfg_f32(1.5f, ctx);

	/* broadcast to match operand shape (no-op for scalar) */
	{
		struct aclutil_mk_constant_repvec_t br;
		br = aclval_cfg_vec_broadcast(u32_like_ty, magic_id, ACLUTIL_LITERAL_MASK_LITERAL, ctx);
		magic_id = br.m_id;
		br = aclval_cfg_vec_broadcast(u32_like_ty, one_id, ACLUTIL_LITERAL_MASK_LITERAL, ctx);
		one_id = br.m_id;
		br = aclval_cfg_vec_broadcast(f32_like_ty, half_id, ACLUTIL_LITERAL_MASK_LITERAL, ctx);
		half_id = br.m_id;
		br = aclval_cfg_vec_broadcast(f32_like_ty, threehalf_id, ACLUTIL_LITERAL_MASK_LITERAL, ctx);
		threehalf_id = br.m_id;
	}

	x_id = rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_END];

	rdwr_prv_extra->m_type_original = f32_like_ty;

	if (rdwr_lst_extra[0].m_mask_literal & (aclspv_wrd_t)ACLUTIL_LITERAL_LITERAL) {
		aclutil_literal res = rsqrtf_nr(rdwr_lst_extra[0].m_literal);
		*r_ltrl = res;
		rdwr_prv_extra->m_literal = res;
		rdwr_prv_extra->m_mask_literal = ACLUTIL_LITERAL_MASK_LITERAL;

		ae2f_ifezerr(*rdwr_prv_idval = aclval_cfg_f32(res.m_flt, ctx)) {
			return ACLEXPR_CARVE_NO;
		}
		return ACLEXPR_CARVE_OK;
	}

	/* non-literal: emit NR sequence as runtime ops (constant-folding only for LITERAL case) */
	rdwr_prv_extra->m_mask_literal = 0;

	aclspv_id_t x_half_id, x_u32_id, shifted_id, startup_u32_id;

	/* x_half = x * 0.5 */
	x_half_id = aclcur_cfg_op2(
			0, SpvOpFMul, f32_like_ty, x_id, half_id, ctx);
	ae2f_ifezerr(x_half_id) return ACLEXPR_CARVE_NO;

	/* x_u32 = bitcast(x) */
	x_u32_id = aclcur_cfg_op1(
			0, SpvOpBitcast, u32_like_ty, x_id, ctx);
	ae2f_ifezerr(x_u32_id) return ACLEXPR_CARVE_NO;

	/* shifted = x_u32 >> 1 */
	shifted_id = aclcur_cfg_op2(
			0, SpvOpShiftRightLogical, u32_like_ty, x_u32_id, one_id, ctx);
	ae2f_ifezerr(shifted_id) return ACLEXPR_CARVE_NO;

	/* startup_u32 = magic - shifted */
	startup_u32_id = aclcur_cfg_op2(
			0, SpvOpISub, u32_like_ty, magic_id, shifted_id, ctx);
	ae2f_ifezerr(startup_u32_id) return ACLEXPR_CARVE_NO;

	/* y = bitcast(startup) */
	aclspv_id_t y_id = aclcur_cfg_op1(
			0, SpvOpBitcast, f32_like_ty, startup_u32_id, ctx);
	ae2f_ifezerr(y_id) return ACLEXPR_CARVE_NO;

	/* unrolled NR iters: y = y * (1.5 - (x_half * y*y)) */
	int it;
	for (it = __ACLSPV_MATH_NR_IT; it > 0; --it) {
		aclspv_id_t y2 = aclcur_cfg_op2(
				0, SpvOpFMul, f32_like_ty, y_id, y_id, ctx);
		ae2f_ifezerr(y2) return ACLEXPR_CARVE_NO;

		aclspv_id_t tmp = aclcur_cfg_op2(
				0, SpvOpFMul, f32_like_ty, x_half_id, y2, ctx);
		ae2f_ifezerr(tmp) return ACLEXPR_CARVE_NO;

		aclspv_id_t sub = aclcur_cfg_op2(
				0, SpvOpFSub, f32_like_ty, threehalf_id, tmp, ctx);
		ae2f_ifezerr(sub) return ACLEXPR_CARVE_NO;

		y_id = aclcur_cfg_op2(
				0, SpvOpFMul, f32_like_ty, y_id, sub, ctx);
		ae2f_ifezerr(y_id) return ACLEXPR_CARVE_NO;
	}

	*rdwr_prv_idval = y_id;
	*r_ltrl = rdwr_prv_extra->m_literal; /* zeroed */
	return ACLEXPR_CARVE_OK;
}
#else
typedef int dummy_t;
#endif
