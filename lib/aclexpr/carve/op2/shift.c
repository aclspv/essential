#include "./shift.h"

#include <aclty.h>

#include <aclexpr/carve/util.h>
#include <aclexpr/carve/lvalue.h>
#include <aclexpr/curreg.h>

#include <aclcur/common.h>
#include <aclconst/specop.h>
#include <aclmem.h>

#include <util/wrdemit.h>
#include <ae2f/cc/switchctl.h>
#include <ae2f/pat/dbg.h>


ae2f_inline_f static ae2f_ccconst aclspv_bool_t s_is_shift_assign(const aclspv_wrd_t x) {
	switch(x) {
		default:
			return 0;

		case ACLEXPR_OP2_SHLASS:
		case ACLEXPR_OP2_SHRASS:
			return 1;
	}
}

ae2f_inline_f static ae2f_ccpure aclspv_opcode_t s_shift_to_spv(
		const aclspv_id_t	c_id,
		const aclspv_wrd_t	c_op2,
		const h_aclspv_ctx_t	h_ctx
		)
{
	aclspv_id_t ID = aclty_simd_class(c_id, h_ctx);

	switch(c_op2) {
		default:
			return 0;

		case ACLEXPR_OP2_SHL:
		case ACLEXPR_OP2_SHLASS:
			if(aclid_is_int(ID))
				return SpvOpShiftLeftLogical;
			else return 0;

		case ACLEXPR_OP2_SHR:
		case ACLEXPR_OP2_SHRASS:
			if(aclid_is_sint(ID))
				return SpvOpShiftRightArithmetic;
			else if(aclid_is_uint(ID))
				return SpvOpShiftRightLogical;
			else return 0;
	}
}

aclspv_bool_t aclexpr_carve2_shift(
		struct aclexpr_visit_data	ae2fcc_arg_sarr_restrict(rdwr_data, 1),
		aclspv_wrd_t			ae2fcc_arg_sarr_restrict(rdwr_prv_idval, 1),
		aclexpr_opbincfg_extra		ae2fcc_arg_sarr_restrict(rdwr_prv_extra, 1),
		aclspv_wrd_t			ae2fcc_arg_sarr_restrict(rdwr_lst_scale_buf, ACLEXPR_OPBINCFG_END + 2),
		aclexpr_opbincfg_extra		ae2fcc_arg_sarr_restrict(rdwr_lst_extra, 2)
		) {
	const aclspv_bool_t IS_ASSIGN = s_is_shift_assign(rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_OPCODE]);

	/** C99 6.5.7: the result of a shift has the (promoted) type of the LEFT
	 * operand — never the common type of both operands. decide_cast would
	 * mix in the shift count's type (mis-signing a `u32 << int`), and the
	 * compound-assign store gate demands result == register type. */
	rdwr_prv_extra->m_type_original = rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_RESTYID]
		= ae2f_expected_strict(aclty_is_simd(rdwr_lst_extra[0].m_type_original, rdwr_data->m_ctx))
		? rdwr_lst_extra[0].m_type_original 
		: 0;

	ae2f_ifezerr(rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_RESTYID]) {
		err_puts("[carve2::shift] no result type");
		err_prefix(0);
		err_call((void), fprintf(stderr, "[carve2::shift] opcode: %u %u\n"
					, (unsigned)rdwr_lst_extra[0].m_type_original
					, (unsigned)rdwr_lst_extra[1].m_type_original));
		rdwr_data->m_ctx->m_err = ACLSPV_COMPILE_UNHANDLED_CARVE;
		return ACLEXPR_CARVE_NO;
	}

	rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_RESID]	= *rdwr_prv_idval = CTX(rdwr_data)->m_id++;

	rdwr_prv_extra->m_mask_literal = rdwr_lst_extra[0].m_mask_literal & rdwr_lst_extra[1].m_mask_literal;

	/** 
	 * casts each operand into the decided result type before emitting the
	 * shift node; the cast ids are allocated in OPITER(1) then OPITER(0)
	 * order to keep the RESID/id order identical to the arithmetic carve.
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
			dbg_call((void), fprintf(stderr, "SHIFT CAST ON LOOP[%u]: %u %u\n"			\
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
														err_puts("[carve2::arithmetic_cast] OPITER"); return ACLEXPR_CARVE_NO;							\
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

	/** 
	 * shifts are CURRENTLY always runtime graph nodes: the constant/specop
	 * fold for shifts is a follow-up (operand masks already arrive fold-ready
	 * via the DeclRefExpr / served-value work, so the structure supports it
	 * later) — no fold, no SpecConstantOp, no s_eval2_ltrl copy here.
	 * */
	rdwr_prv_extra->m_mask_literal = ACLUTIL_LITERAL_MASK_RT;

	ae2f_ifezerr(rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_OPCODE] = s_shift_to_spv(
				ACLEXPR_OPBINCFG_RESTYID[rdwr_lst_scale_buf]
				, (ACLEXPR_OPBINCFG_OPCODE[rdwr_lst_scale_buf])
				, CTX(rdwr_data)
				)) {
		err_puts("[carve2::shift] SpvOp went undefined (shift of non-integer)");
		return ACLEXPR_CARVE_NO;
	} else {
		dbg_prefix(a);
		dbg_call((void), (fprintf(stderr, "SpvOp_[%u]\n", rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_OPCODE])));
	}

	ae2f_ifnezerr_strict(rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_END] >= rdwr_data->m_ctx->m_id)
		return ACLEXPR_CARVE_NO;

	ae2f_ifnezerr_strict(rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_END + 1] >= rdwr_data->m_ctx->m_id)
		return ACLEXPR_CARVE_NO;

	{
		/** the result type is the PLAIN common integer type — no bool
		 * swap like the comparisons. */
		const aclspv_id_t TY_RES = rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_RESTYID];

		ae2f_ifezerr(*rdwr_prv_idval = aclcur_cfg_op2(
					rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_RESID]
					, (enum SpvOp_)rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_OPCODE]
					, TY_RES
					, rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_END]
					, rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_END + 1]
					, rdwr_data->m_ctx)) {
			err_puts("[carve2::shift] aclcur_cfg_op2");
			return ACLEXPR_CARVE_NO;
		}
	}

	if(IS_ASSIGN) {
		/** view-rooted (`a[i] <<= x`), vector element (`v[i] <<= x`) or
		 * plain variable: one shared store-back, keyed on what the l-value's
		 * own read recorded on extra[0]. */
		ae2f_ifezerr(aclexpr_lvalue_storeback(
					rdwr_lst_extra
					, *rdwr_prv_idval
					, rdwr_data->m_ctx
					, "[carve2::shift] the l-value store back failed")) {
			return ACLEXPR_CARVE_NO;
		}
	}
	return ACLEXPR_CARVE_OK;
}

#undef	OPITER
