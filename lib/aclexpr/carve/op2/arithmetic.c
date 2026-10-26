#include "./arithmetic.h"

#include <aclexpr/carve/util.h>
#include <aclexpr/carve/ptr.h>
#include <aclexpr/carve/lvalue.h>
#include <aclcur/common.h>
#include <aclcur/op.h>
#include <aclexpr/curreg.h>
#include <aclconst/specop.h>
#include <aclmem.h>

#include <util/wrdemit.h>
#include <ae2f/cc/switchctl.h>
#include <ae2f/pat/dbg.h>

enum S_ARITHMETIC_ {
	S_ARITHMETIC_ADD	= ACLEXPR_OP2_ADD,
	S_ARITHMETIC_ADDX	= ACLEXPR_OP2_ADDASS,
	S_ARITHMETIC_SUB	= ACLEXPR_OP2_SUB,
	S_ARITHMETIC_SUBX	= ACLEXPR_OP2_SUBASS,
	S_ARITHMETIC_MUL	= ACLEXPR_OP2_MUL,
	S_ARITHMETIC_MULX	= ACLEXPR_OP2_MULASS,
	S_ARITHMETIC_DIV	= ACLEXPR_OP2_DIV,
	S_ARITHMETIC_DIVX	= ACLEXPR_OP2_DIVASS,
	S_ARITHMETIC_REM	= ACLEXPR_OP2_REM,
	S_ARITHMETIC_REMX	= ACLEXPR_OP2_REMASS
};

ae2f_inline_f static ae2f_ccconst aclspv_bool_t s_is_assign(aclspv_wrd_t c_opcode) {

	switch(c_opcode) {
		case S_ARITHMETIC_ADD:
		case S_ARITHMETIC_DIV:
		case S_ARITHMETIC_MUL:
		case S_ARITHMETIC_REM:
		case S_ARITHMETIC_SUB:
		default:
			return 0;

		case S_ARITHMETIC_ADDX:
		case S_ARITHMETIC_DIVX:
		case S_ARITHMETIC_MULX:
		case S_ARITHMETIC_SUBX:
		case S_ARITHMETIC_REMX:
			return 1;
	}
}

ae2f_inline_f static ae2f_ccpure aclspv_opcode_t s_op_to_spv(
		const aclspv_id_t c_id,
		const aclspv_wrd_t c_op2,
		const h_aclspv_ctx_t	h_ctx
		)
{
	aclspv_id_t	ID = aclty_simd_class(c_id, h_ctx);

	switch((enum S_ARITHMETIC_)c_op2) {
		default:
			return 0;

		case S_ARITHMETIC_ADD:
		case S_ARITHMETIC_ADDX:
			if(aclid_is_int(ID))
				return SpvOpIAdd;
			else if(aclid_is_float(ID))
				return SpvOpFAdd;
			else return 0;

		case S_ARITHMETIC_DIV:
		case S_ARITHMETIC_DIVX:
			if(aclid_is_sint(ID))
				return SpvOpSDiv;
			else if (aclid_is_uint(ID))
				return SpvOpUDiv;
			else if(aclid_is_float(ID))
				return SpvOpFDiv;
			else return 0;

		case S_ARITHMETIC_MUL:
		case S_ARITHMETIC_MULX:
			if(aclid_is_sint(ID))
				return SpvOpIMul;
			else if (aclid_is_uint(ID))
				return SpvOpIMul;
			else if(aclid_is_float(ID))
				return SpvOpFMul;
			else return 0;

		case S_ARITHMETIC_REM:
		case S_ARITHMETIC_REMX:
			if(aclid_is_sint(ID))
				return SpvOpSRem;
			else if (aclid_is_uint(ID))
				return SpvOpUMod;
			else if(aclid_is_float(ID))
				return SpvOpFMod;
			else return 0;

		case S_ARITHMETIC_SUB:
		case S_ARITHMETIC_SUBX:
			if(aclid_is_sint(ID))
				return SpvOpISub;
			else if (aclid_is_uint(ID))
				return SpvOpISub;
			else if(aclid_is_float(ID))
				return SpvOpFSub;
			else return 0;
	}
}

ae2f_inline_f static ae2f_ccconst struct aclutil_ltrl_eval_t	s_eval2_ltrl(
		const aclutil_literal	c_oprndA,
		const aclutil_literal	c_oprndB,
		const enum SpvOp_	c_opcode,
		const enum ACLSPV_ID_	c_type
		)
{
	struct aclutil_ltrl_eval_t RET;

	switch((uintmax_t)c_type) {
#define	EXPR_TWO(L_oper, L_mem, c_case)					\
		case	c_case:						\
									\
		RET.m_ret.L_mem = c_oprndA.L_mem L_oper c_oprndB.L_mem;	\
		RET.m_found = 1					\

		case ACLSPV_ID_I8:
		case ACLSPV_ID_I16:
		case ACLSPV_ID_I32:
		case ACLSPV_ID_I64:
			switch((uintmax_t)c_opcode) {
				EXPR_TWO(+, m_api_intmax, SpvOpIAdd); return RET;
				EXPR_TWO(-, m_api_intmax, SpvOpISub); return RET;
				EXPR_TWO(*, m_api_intmax, SpvOpIMul); return RET;
				EXPR_TWO(/, m_api_intmax, SpvOpSDiv); return RET;
				EXPR_TWO(/, m_api_intmax, SpvOpUDiv); return RET;
				EXPR_TWO(%, m_api_intmax, SpvOpSRem); return RET;

				case SpvOpSLessThan:
				RET.m_ret.m_api_intmax = c_oprndA.m_api_intmax < c_oprndB.m_api_intmax;
				RET.m_found = 1; return RET;
				case SpvOpSLessThanEqual:
				RET.m_ret.m_api_intmax = c_oprndA.m_api_intmax <= c_oprndB.m_api_intmax;
				RET.m_found = 1; return RET;
				case SpvOpSGreaterThan:
				RET.m_ret.m_api_intmax = c_oprndA.m_api_intmax > c_oprndB.m_api_intmax;
				RET.m_found = 1; return RET;
				case SpvOpSGreaterThanEqual:
				RET.m_ret.m_api_intmax = c_oprndA.m_api_intmax >= c_oprndB.m_api_intmax;
				RET.m_found = 1; return RET;
				case SpvOpIEqual:
				RET.m_ret.m_api_intmax = c_oprndA.m_api_intmax == c_oprndB.m_api_intmax;
				RET.m_found = 1; return RET;
				case SpvOpINotEqual:
				RET.m_ret.m_api_intmax = c_oprndA.m_api_intmax != c_oprndB.m_api_intmax;
				RET.m_found = 1; return RET;

				default:
				RET.m_ret.m_u64 = 0;
				RET.m_found = 0;
				return RET;
			}

		case ACLSPV_ID_U8:
		case ACLSPV_ID_U16:
		case ACLSPV_ID_U32:
		case ACLSPV_ID_U64:
			switch((uintmax_t)c_opcode) {
				EXPR_TWO(+, m_api_uintmax, SpvOpIAdd); return RET;
				EXPR_TWO(-, m_api_uintmax, SpvOpISub); return RET;
				EXPR_TWO(*, m_api_uintmax, SpvOpIMul); return RET;
				EXPR_TWO(/, m_api_uintmax, SpvOpSDiv); return RET;
				EXPR_TWO(/, m_api_uintmax, SpvOpUDiv); return RET;
				EXPR_TWO(%, m_api_uintmax, SpvOpSRem); return RET;

				case SpvOpSLessThan:
				RET.m_ret.m_api_uintmax = c_oprndA.m_api_uintmax < c_oprndB.m_api_uintmax;
				RET.m_found = 1; return RET;
				case SpvOpSLessThanEqual:
				RET.m_ret.m_api_uintmax = c_oprndA.m_api_uintmax <= c_oprndB.m_api_uintmax;
				RET.m_found = 1; return RET;
				case SpvOpSGreaterThan:
				RET.m_ret.m_api_uintmax = c_oprndA.m_api_uintmax > c_oprndB.m_api_uintmax;
				RET.m_found = 1; return RET;
				case SpvOpSGreaterThanEqual:
				RET.m_ret.m_api_uintmax = c_oprndA.m_api_uintmax >= c_oprndB.m_api_uintmax;
				RET.m_found = 1; return RET;
				case SpvOpIEqual:
				RET.m_ret.m_api_uintmax = c_oprndA.m_api_uintmax == c_oprndB.m_api_uintmax;
				RET.m_found = 1; return RET;
				case SpvOpINotEqual:
				RET.m_ret.m_api_uintmax = c_oprndA.m_api_uintmax != c_oprndB.m_api_uintmax;
				RET.m_found = 1; return RET;

				default:
				RET.m_ret.m_u64 = 0;
				RET.m_found = 0;
				return RET;
			}

		case ACLSPV_ID_F32:
			switch((uintmax_t)c_opcode) {
				EXPR_TWO(+, m_flt, SpvOpFAdd); return RET;
				EXPR_TWO(-, m_flt, SpvOpFSub); return RET;
				EXPR_TWO(/, m_flt, SpvOpFDiv); return RET;
				EXPR_TWO(*, m_flt, SpvOpFMul); return RET;

				default:
				RET.m_ret.m_u64 = 0;
				RET.m_found = 0;
				return RET;
			}

		case ACLSPV_ID_F64:
			switch((uintmax_t)c_opcode) {
				EXPR_TWO(+, m_dbl, SpvOpFAdd); return RET;
				EXPR_TWO(-, m_dbl, SpvOpFSub); return RET;
				EXPR_TWO(/, m_dbl, SpvOpFDiv); return RET;
				EXPR_TWO(*, m_dbl, SpvOpFMul); return RET;
				default:
				RET.m_ret.m_u64 = 0;
				RET.m_found = 0;
				return RET;
			}
		default:
			RET.m_ret.m_u64 = 0;
			RET.m_found = 0;
			return RET;
	}
	ae2f_unreachable();

#undef	EXPR_TWO
}

static aclspv_wrd_t s_decide_cast(aclspv_id_t c_id0, aclspv_id_t c_id1, const h_aclspv_ctx_t h_ctx) {
	aclspv_id_t cls0, cls1; 

	if(c_id0 == c_id1)
		return c_id0;

	/** sorting to make it simpler */
	if(c_id1 < c_id0) {
		aclspv_id_t tmp;
		tmp = c_id0;
		c_id0 = c_id1;
		c_id1 = tmp;
	}

	cls0 = aclty_simd_class(c_id0, h_ctx);
	cls1 = aclty_simd_class(c_id1, h_ctx);

	/** one is 0? */
	unless(c_id0)
		return c_id1;

	ae2f_ifezerr(cls0 && cls1)
		return 0;

	if(aclty_simd_chxg_cls(c_id0, cls1, h_ctx) != c_id1) {
		/** shape is not same. if */	
		ae2f_ifneznrm_strict(c_id0 == cls0) return c_id1;
		return 0;
	}

	/** shape is same */
	return cls0 < cls1 ? c_id1 : c_id0;
}

#include <ae2f/cc/arg_sarr.h>

/** 0 when succeed */
aclspv_bool_t aclexpr_carve2_arithmetic_do(
		aclutil_literal			ae2fcc_arg_sarr_restrict(r_ltrl, 1),
		aclexpr_opbincfg_extra		ae2fcc_arg_sarr_restrict(r_extra, 1),
		aclspv_id_t			ae2fcc_arg_sarr_restrict(r_id, 1),
		const aclexpr_opbincfg_extra	ae2fcc_arg_sarr_restrict(rd_extra, 2),
		aclspv_wrd_t			ae2fcc_arg_sarr_restrict(rdwr_lst_scale_buf, ACLEXPR_OPBINCFG_END + 2),
		h_aclspv_ctx_t			h_ctx
		) {
	if(
			aclid_is_float(aclty_simd_class(rd_extra[0].m_type_original, h_ctx))
			|| aclid_is_float(aclty_simd_class(rd_extra[1].m_type_original, h_ctx))
	  ) {
#if	__ACLSPV_OpCapabilityKernel
		if(aclutil_add_capability_by_flag(h_ctx, SpvCapabilityKernel))
#endif
		{
			fixme_puts("this is float. make sure we can do it constantly with Kernel capability.");
			/** preserve LITERAL path for constant folding (aclval_cfg_by_enum)
			 * so that OpConstant is emitted in dominating section instead of
			 * OpFSub inside one if-arm only. Fixes dominance error in
			 * chained if-else (t16_vs_main / gl-simple test). */
			if ((r_extra->m_mask_literal & ACLUTIL_LITERAL_LITERAL) == 0)
				r_extra->m_mask_literal = ACLUTIL_LITERAL_MASK_RT;
			else
				r_extra->m_mask_literal &= ~(aclspv_wrd_t)(ACLUTIL_LITERAL_CONSTANT);
		}
	}

	ae2f_ifezerr(rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_OPCODE] = s_op_to_spv(
				ACLEXPR_OPBINCFG_RESTYID[rdwr_lst_scale_buf]
				, (ACLEXPR_OPBINCFG_OPCODE[rdwr_lst_scale_buf])
				, h_ctx
				)) {
		err_puts(("[carve2::arithmetic] SpvOp went undefined"));
		return 1;
	} else {
		dbg_prefix(a);
		dbg_call((void), (fprintf(stderr, "SpvOp_[%u]\n", rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_OPCODE])));
	}

	ae2f_ifnezerr_strict(rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_END] >= h_ctx->m_id)
		return 1;

	ae2f_ifnezerr_strict(rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_END + 1] >= h_ctx->m_id)
		return 1;

	switch(r_extra->m_mask_literal) {
		case ACLUTIL_LITERAL_MASK_LITERAL:
		case ACLUTIL_LITERAL_LITERAL:
			{
				struct aclutil_ltrl_eval_t EVAL_LIT
					= s_eval2_ltrl(
							rd_extra[0].m_literal
							, rd_extra[1].m_literal
							, (enum SpvOp_)rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_OPCODE]
							,(enum ACLSPV_ID_)rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_RESTYID]
						      );

				ae2f_ifnezerr_strict(EVAL_LIT.m_found) {
					dbg_puts(("The result is literal."));
					*r_ltrl =  r_extra->m_literal = EVAL_LIT.m_ret;
					r_extra->m_type_original = rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_RESTYID];

					ae2f_ifezerr(*r_id = aclval_cfg_by_enum(EVAL_LIT.m_ret.m_api_uintmax, h_ctx
								, (enum ACLSPV_ID_)rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_RESTYID])) {
						err_puts("[carve2::arithmetic] aclutil_mk_constant_val_by_enum");
						return 1;
					}

					dbg_prefix();
					dbg_call((void), fprintf(stderr, "literal id: %u\n", *r_id));
					break;
				} else {
					err_puts("[carve2::arithmetic] EVAL_LIT");
					return 1;
				}
			}

			break;


		case ACLUTIL_LITERAL_MASK_CONSTANT:
			ae2f_ifezerr(*r_id = aclconst_specop(
						rdwr_lst_scale_buf
						, 6
						, h_ctx)) return 1;
			break;

		case ACLUTIL_LITERAL_MASK_RT:
			/** runtime: the operation is a graph node, flushed by aclcur_emit(). */
			{
				aclspv_id_t TY_RES = rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_RESTYID];

				ae2f_ifezerr(*r_id = aclcur_cfg_op2(
							rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_RESID]
							, (enum SpvOp_)rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_OPCODE]
							, TY_RES
							, rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_END]
							, rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_END + 1]
							, h_ctx))
					return 1;
			}

			break;

		default:
			err_puts(("unknown literal"));
			ae2f_unreachable();
	}

	if(s_is_assign(rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_OPCODE])) {
		/** view-rooted (`*q += x`, `p->m += x`, `a[i] += x`), vector element
		 * (`v[i] += x`) or plain variable: one shared store-back, keyed on
		 * what the l-value's own read recorded on extra[0]. */
		ae2f_ifezerr(aclexpr_lvalue_storeback(
					r_extra
					, *r_id
					, h_ctx
					, "[carve2::arithmetic] the l-value store back failed")) {
			return 1;
		}
	}

	return 0;
}

	aclspv_bool_t aclexpr_carve2_arithmetic(
			struct aclexpr_visit_data	ae2fcc_arg_sarr_restrict(rdwr_data, 1),
			aclspv_wrd_t			ae2fcc_arg_sarr_restrict(r_prv_idval, 1),
			aclexpr_opbincfg_extra		ae2fcc_arg_sarr_restrict(rdwr_prv_extra, 1),
			aclspv_wrd_t			ae2fcc_arg_sarr_restrict(rdwr_lst_scale_buf, ACLEXPR_OPBINCFG_END + 2),
			aclexpr_opbincfg_extra		ae2fcc_arg_sarr_restrict(rdwr_lst_extra, 2),
			aclutil_literal			ae2fcc_arg_sarr_restrict(r_ltrl, 1)
			) 
{
	/** pointer arithmetic (`p + n`, `p - n`, `p += n`, `p -= n`, `p - q`):
	 * a fatal error in the logical addressing model, where a pointer is a
	 * plain id with no address to offset. the ACLTY_PTR operand test keeps
	 * ordinary integer / float arithmetic untouched, and the check is
	 * gated on the logical flag alone so physical addressing keeps it. */
	if(aclexpr_ptr_arith_fatal(rdwr_lst_extra[0].m_type_original, CTX(rdwr_data))
			|| aclexpr_ptr_arith_fatal(rdwr_lst_extra[1].m_type_original, CTX(rdwr_data)))
		return ACLEXPR_CARVE_NO;

	rdwr_prv_extra->m_type_original = rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_RESTYID] = s_decide_cast(
			rdwr_lst_extra[0].m_type_original
			, rdwr_lst_extra[1].m_type_original
			, CTX(rdwr_data)
			);

	rdwr_prv_extra->m_mask_literal = rdwr_lst_extra[0].m_mask_literal & rdwr_lst_extra[1].m_mask_literal;


	ae2f_ifezerr(rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_RESTYID]) {
		err_puts("[carve2::arithmetic] no result type");
		err_prefix(0);
		err_call((void), fprintf(stderr, "[carve2::arithmetic] opcode: %u %u\n"
					, (unsigned)rdwr_lst_extra[0].m_type_original
					, (unsigned)rdwr_lst_extra[1].m_type_original));
		return ACLEXPR_CARVE_NO;
	}

	rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_RESID]	= *r_prv_idval = CTX(rdwr_data)->m_id++;

#define	OPITER(NUM_OPRND) \
	{										\
		{	\
			const aclutil_literal OLD_LITERAL = rdwr_lst_extra[NUM_OPRND].m_literal;			\
			aclspv_id_t OLD_ID 									\
			= rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_END + NUM_OPRND];					\
			rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_END + NUM_OPRND] = CTX(rdwr_data)->m_id++;		\
			\
			dbg_prefix(a);										\
			dbg_call((void), fprintf(stderr, "ARITHMETIC CAST ON LOOP[%u]: %u %u\n"			\
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
					, *r_prv_idval, rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_END + NUM_OPRND]	\
					, (void*)&rdwr_lst_extra[NUM_OPRND].m_mask_literal				\
					, rdwr_lst_extra[NUM_OPRND].m_mask_literal					\
					, rdwr_prv_extra->m_mask_literal					\
					));										\
	}

	OPITER(1u);
	OPITER(0u);

	return aclexpr_carve2_arithmetic_do(
			r_ltrl, rdwr_prv_extra
			, r_prv_idval, rdwr_lst_extra
			, rdwr_lst_scale_buf, rdwr_data->m_ctx
			);
}
