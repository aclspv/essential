#include "./cmp.h"

#include <aclty.h>

#include <aclcur/common.h>

#include <aclexpr/curreg.h>
#include <aclexpr/carve/util.h>

#include <aclconst/specop.h>
#include <aclmem.h>

#include <util/wrdemit.h>
#include <ae2f/cc/switchctl.h>
#include <ae2f/pat/dbg.h>
#include <util/constant.h>

static aclspv_wrd_t s_decide_cast(aclspv_id_t c_id0, aclspv_id_t c_id1, const h_aclspv_ctx_t h_ctx) {
	aclspv_id_t cls0, cls1;

	dbg_prefix(a);
	dbg_call(fprintf, (
				stderr, "s_decide_cast(%lu, %lu)\n"
				, (unsigned long)c_id0
				, (unsigned long)c_id1
				));

	if(c_id0 == c_id1)
		return (aclspv_id_t) c_id0;

	/** one is 0? */
	unless(c_id0 && c_id1)
		return c_id0 ? c_id0 : c_id1;

	if(c_id1 > c_id0) {
		c_id0 ^= c_id1;
		c_id1 ^= c_id0;
		c_id0 ^= c_id1;
	}

	cls0 = aclty_simd_class(c_id0, h_ctx);
	cls1 = aclty_simd_class(c_id1, h_ctx);

	ae2f_ifezerr(cls0 && cls1) {
		dbg_puts("s_decide_cast 0\n");
		return 0;
	}

	if(aclty_simd_chxg_cls(c_id0, cls1, h_ctx) != c_id1) {
		ae2f_ifneznrm(aclty_is_simd_class(c_id0))
			return c_id1;
		else	return 0;
	}

	return cls0 < cls1 ? c_id1 : c_id0;
}

ae2f_inline_f static aclspv_id_t s_ty_cmp_result(
		const aclspv_id_t	c_ty,
		const h_aclspv_ctx_t	h_ctx
		) {
	return aclty_simd_chxg_cls(c_ty, ACLSPV_ID_BOOL, h_ctx);
}

ae2f_inline_f static ae2f_ccconst aclspv_bool_t s_is_cmp_op(const aclspv_wrd_t x) {
	switch(x) {
		default:
			return 0;

		case ACLEXPR_OP2_SLT:
		case ACLEXPR_OP2_SLE:
		case ACLEXPR_OP2_SGT:
		case ACLEXPR_OP2_SGE:
		case ACLEXPR_OP2_EQ:
		case ACLEXPR_OP2_NE:
			return 1;
	}
}

#define CHECK unless(	\
		s_is_cmp_op(	\
			rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_OPCODE]	\
			)) return c_lbl_continue

ae2f_inline_f static ae2f_ccpure aclspv_opcode_t s_cmp_to_spv(
		const aclspv_id_t	c_id,
		const aclspv_wrd_t	c_op2,
		const h_aclspv_ctx_t	h_ctx
		)
{
	aclspv_id_t ID = aclty_simd_class(c_id, h_ctx);

	switch(c_op2) {
		default:
			return 0;

		case ACLEXPR_OP2_SLT:
			if(aclid_is_int(ID))
				return SpvOpSLessThan;
			else if(aclid_is_float(ID))
				return SpvOpFOrdLessThan;
			else return 0;

		case ACLEXPR_OP2_SLE:
			if(aclid_is_int(ID))
				return SpvOpSLessThanEqual;
			else if(aclid_is_float(ID))
				return SpvOpFOrdLessThanEqual;
			else return 0;

		case ACLEXPR_OP2_SGT:
			if(aclid_is_int(ID))
				return SpvOpSGreaterThan;
			else if(aclid_is_float(ID))
				return SpvOpFOrdGreaterThan;
			else return 0;

		case ACLEXPR_OP2_SGE:
			if(aclid_is_int(ID))
				return SpvOpSGreaterThanEqual;
			else if(aclid_is_float(ID))
				return SpvOpFOrdGreaterThanEqual;
			else return 0;

		case ACLEXPR_OP2_EQ:
			if(aclid_is_int(ID))
				return SpvOpIEqual;
			else if(aclid_is_float(ID))
				return SpvOpFOrdEqual;
			else return 0;

		case ACLEXPR_OP2_NE:
			if(aclid_is_int(ID))
				return SpvOpINotEqual;
			else if(aclid_is_float(ID))
				return SpvOpFOrdNotEqual;
			else return 0;
	}
}

aclexpr_carve_lbl_t aclexpr_carve2_cmp(
		struct aclexpr_visit_data* const ae2f_restrict	rdwr_data,
		aclspv_wrd_t* const ae2f_restrict		rdwr_prv_idval,
		aclexpr_opbincfg_extra* const ae2f_restrict	rdwr_prv_extra,
		ae2f_unused h_scale_t				h_lst_scale,
		aclspv_wrd_t* const ae2f_restrict		rdwr_lst_scale_buf,
		aclexpr_opbincfg_extra* ae2f_restrict		rdwr_lst_extra,
		ae2f_unused aclutil_literal* ae2f_restrict	r_ltrl,
		aclexpr_carve_lbl_t				c_lbl_continue,
		aclexpr_carve_lbl_t				c_lbl_success,
		aclexpr_carve_lbl_t				c_lbl_err
		) { CHECK; {

	rdwr_prv_extra->m_type_original = rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_RESTYID] = s_decide_cast(
			rdwr_lst_extra[0].m_type_original
			, rdwr_lst_extra[1].m_type_original
			, CTX(rdwr_data)
			);

	rdwr_prv_extra->m_mask_literal = rdwr_lst_extra[0].m_mask_literal & rdwr_lst_extra[1].m_mask_literal;

	ae2fpat_assertassume_strict(rdwr_lst_scale_buf);

	ae2f_ifezerr(rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_RESTYID]) {
		err_puts("[carve2::cmp] no result type");
		err_prefix(0);
		err_call((void), fprintf(stderr, "[carve2::cmp] opcode: %u %u\n"
					, (unsigned)rdwr_lst_extra[0].m_type_original
					, (unsigned)rdwr_lst_extra[1].m_type_original));
		rdwr_data->m_ctx->m_err = ACLSPV_COMPILE_UNHANDLED_CARVE;
		return c_lbl_err;
	}

	rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_RESID]	= *rdwr_prv_idval = CTX(rdwr_data)->m_id++;

	/** 
	 * casts each operand into the decided result type before emitting the
	 * compare node; the cast ids are allocated in OPITER(1) then OPITER(0)
	 * order to keep the RESID/id order identical to the old arithmetic carve.
	 * */
#define	OPITER(NUM_OPRND) \
	{										\
		{	\
			const aclutil_literal OLD_LITERAL = rdwr_lst_extra[NUM_OPRND].m_literal;			\
			aclspv_id_t OLD_ID 									\
			= rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_END + NUM_OPRND];					\
			rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_END + NUM_OPRND] = CTX(rdwr_data)->m_id++;		\
			\
			dbg_prefix(a);										\
			dbg_call((void), fprintf(stderr, "COMPARE CAST ON LOOP[%u]: %u %u\n"			\
						, NUM_OPRND							\
						, OLD_ID, rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_END + NUM_OPRND]	\
						));								\
			\
			switch(aclexpr_arithmetic_cast(								\
						OLD_ID								\
						, rdwr_lst_extra[NUM_OPRND].m_type_original			\
						, rdwr_lst_extra[NUM_OPRND].m_mask_literal			\
						, OLD_LITERAL							\
						, &rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_END + NUM_OPRND]		\
						, rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_RESTYID]			\
						, &rdwr_lst_extra[NUM_OPRND].m_literal				\
						, CTX(rdwr_data)						\
						))								\
			{											\
				default:									\
				case ACLEXPR_RET_FAILURE:								\
				case ACLEXPR_RET_NOT_THE_CASE:							\
														err_puts("[carve2::arithmetic_cast] OPITER"); return c_lbl_err;							\
				\
				case ACLEXPR_RET_SUCCESS:								\
															rdwr_prv_extra->m_mask_literal &= 0;			\
				ae2f_fallthrough;							\
				case ACLEXPR_RET_SUCCESS_CONSTANT:						\
														rdwr_prv_extra->m_mask_literal &= 1;			\
				ae2f_fallthrough;							\
				case ACLEXPR_RET_SUCCESS_LITERAL:							\
															break;									\
			}											\
		}	\
		\
		dbg_prefix(i);											\
		dbg_call((void), fprintf(stderr, "deepbin2-%u[%u]{%u} and literal mask %p [%u]{%u}\n"		\
					, NUM_OPRND	\
					, *rdwr_prv_idval, rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_END + NUM_OPRND]	\
					, (void*)&rdwr_lst_extra[NUM_OPRND].m_mask_literal				\
					, rdwr_lst_extra[NUM_OPRND].m_mask_literal					\
					, rdwr_prv_extra->m_mask_literal					\
					));										\
	}

	OPITER(1u);
	OPITER(0u);

	/** comparisons produce a bool; the const/specop templates (OpConstant /
	 * OpSpecConstantOp) cannot carry a bool result type cleanly, so a
	 * comparison is ALWAYS a runtime graph node — regardless of the
	 * operands being literal/constant foldable. */
	rdwr_prv_extra->m_mask_literal = ACLUTIL_LITERAL_MASK_RT;

	ae2f_ifezerr(rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_OPCODE] = s_cmp_to_spv(
				ACLEXPR_OPBINCFG_RESTYID[rdwr_lst_scale_buf]
				, (ACLEXPR_OPBINCFG_OPCODE[rdwr_lst_scale_buf])
				, CTX(rdwr_data)
				)) {
		err_puts("[carve2::cmp] SpvOp went undefined (non-int/non-float compare)");
		return c_lbl_err;
	} else {
		dbg_prefix(a);
		dbg_call((void), (fprintf(stderr, "SpvOp_[%u]\n", rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_OPCODE])));
	}

	ae2f_ifnezerr_strict(rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_END] >= rdwr_data->m_ctx->m_id)
		return c_lbl_err;

	ae2f_ifnezerr_strict(rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_END + 1] >= rdwr_data->m_ctx->m_id)
		return c_lbl_err;

	{
		aclspv_id_t TY_RES = rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_RESTYID];
		TY_RES = s_ty_cmp_result(TY_RES, rdwr_data->m_ctx);

		ae2f_ifezerr(aclid_mk_default_id(ACLSPV_ID_BOOL, rdwr_data->m_ctx))
			return c_lbl_err;

		rdwr_prv_extra->m_type_original = TY_RES;

		ae2f_ifezerr(*rdwr_prv_idval = aclcur_cfg_op2(
					rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_RESID]
					, (enum SpvOp_)rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_OPCODE]
					, TY_RES
					, rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_END]
					, rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_END + 1]
						, rdwr_data->m_ctx))
			return c_lbl_err;
	}

	return c_lbl_success;
}}

#undef	CHECK
#undef	OPITER

