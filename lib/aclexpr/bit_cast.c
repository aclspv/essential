#include <aclexpr.h>
#include <aclty.h>
#include <aclty/simd/cls.h>
#include <aclutil/capability.h>
#include <aclcur/op.h>

#include <ae2f/pat/dbg.h>
#include <ae2f/cc/arg_sarr.h>
#include <aclconst/specop.h>

#include <aclctx/err.h>
#include <aclctx/id.h>

/** c_old_type and c_new_type's shape MUST BE SAME */
static enum ACLEXPR_RET_ s_cast_bit(
		const aclspv_id_t	c_old_id,
		const aclspv_id_t	c_old_type,
		ae2f_unused const aclutil_ltrlmask_t	c_flag_literal,
		aclspv_id_t			ae2fcc_arg_sarr_restrict(r_newid, 1),
		aclspv_id_t				c_new_type,
		const h_aclspv_ctx_t			h_ctx
		) {
	aclconst_specsz old_sz = aclty_getsize(c_old_type, h_ctx);
	aclconst_specsz new_sz = aclty_getsize(c_new_type, h_ctx);

	ae2fpat_assertassume(h_ctx);

	if(old_sz.m_id) return ACLEXPR_RET_FAILURE;
	if(new_sz.m_id) return ACLEXPR_RET_FAILURE;
	ae2f_ifnezerr_strict(old_sz.m_lit.m_max != new_sz.m_lit.m_max)
		return ACLEXPR_RET_FAILURE;

#if	__ACLSPV_OpCapabilityKernel
	if((c_flag_literal & ACLUTIL_LITERAL_CONSTANT)) {
		aclspv_wrd_t buf[5];
		buf[0] = aclspv_opcode_to_wrd(SpvOpSpecConstantOp, 4);
		buf[1] = c_new_type;
		buf[3] = SpvOpBitcast;
		buf[4] = c_old_id;

		ae2f_ifezerr(aclutil_add_capability_by_flag(h_ctx, SpvCapabilityKernel))
			return ACLEXPR_RET_FAILURE;

		ae2f_ifezerr(*r_newid = aclconst_specop(buf, sizeof(buf) / sizeof(buf[0]), h_ctx))
			return ACLEXPR_RET_FAILURE;

		return ACLEXPR_RET_SUCCESS_CONSTANT;
	}
#endif

	ae2f_ifezerr(*r_newid = aclcur_cfg_op1(
				aclctx_newid(h_ctx)
				, SpvOpBitcast
				, c_new_type
				, c_old_id
				, h_ctx))
		return ACLEXPR_RET_FAILURE;

	return ACLEXPR_RET_SUCCESS;
}

enum ACLEXPR_RET_ aclexpr_bit_cast(
		const aclspv_id_t	c_old_id,
		const aclspv_id_t	c_old_type,
		const aclutil_ltrlmask_t	c_flag_literal,
		const aclexpr_literal	c_old_literal,

		aclspv_id_t* ae2f_restrict	r_newid,
		aclspv_id_t		c_new_type,
		aclexpr_literal* ae2f_restrict r_new_literal_opt,
		const h_aclspv_ctx_t	h_ctx
		)
{
	ae2fpat_assertassume(r_newid);
	ae2fpat_assertassume(h_ctx);

	/** two are same */
	if(c_old_type == c_new_type) {
		const uint_fastenum_t	CND_LIT = c_flag_literal & ACLUTIL_LITERAL_LITERAL;
		*r_newid = c_old_id;

		if(r_new_literal_opt)
			*r_new_literal_opt = c_old_literal;

		if (CND_LIT)
			return ACLEXPR_RET_SUCCESS_LITERAL;
		return (c_flag_literal & ACLUTIL_LITERAL_CONSTANT) ? ACLEXPR_RET_SUCCESS_CONSTANT : ACLEXPR_RET_SUCCESS;
	}

	ae2f_ifezerr(c_old_type) {
		_aclctx_err(h_ctx, ACLSPV_COMPILE_ARITHMETIC_CAST_TYPE_IS_UNDEFINED_OLD);
		return ACLEXPR_RET_FAILURE;
	}

	ae2f_ifezerr(c_new_type) {
		_aclctx_err(h_ctx, ACLSPV_COMPILE_ARITHMETIC_CAST_TYPE_IS_UNDEFINED_NEW);
		return ACLEXPR_RET_FAILURE;
	}

	ae2f_ifezerr(c_old_id) {
		_aclctx_err(h_ctx, ACLSPV_COMPILE_ARITHMETIC_CAST_TYPE_IS_UNDEFINED_OLD);
		return ACLEXPR_RET_FAILURE;
	}

	ae2f_ifnezerr(!aclty_is_simd(c_old_type, h_ctx) || !aclty_is_simd(c_new_type, h_ctx)) {
		warn_puts("no arithmetic");
		return ACLEXPR_RET_NOT_THE_CASE;
	}

	(void)aclid_mk_default_id(c_new_type, h_ctx);
	(void)aclid_mk_default_id(c_old_type, h_ctx);


	dbg_prefix();
	dbg_call((void), fprintf(stderr, "[bit_cast] (%u : %u) -> (? : %u)\n"
				, (unsigned) c_old_id
				, (unsigned) c_old_type
				, (unsigned) c_new_type
				));

	unless(aclty_is_simd_class(c_old_type) && aclty_is_simd_class(c_new_type))
		return s_cast_bit(c_old_id, c_old_type, c_flag_literal, r_newid, c_new_type, h_ctx);

	if(c_flag_literal & ACLUTIL_LITERAL_LITERAL) {
		dbg_puts("literal\n");
		*r_new_literal_opt = c_old_literal;
		return ae2f_expected(*r_newid = aclval_cfg_by_enum(c_old_literal.m_u64, h_ctx, (enum ACLSPV_ID_)c_new_type))
			? ACLEXPR_RET_SUCCESS_LITERAL
			: ACLEXPR_RET_FAILURE;
	} else return s_cast_bit(c_old_id, c_old_type, c_flag_literal, r_newid, c_new_type, h_ctx);
}
