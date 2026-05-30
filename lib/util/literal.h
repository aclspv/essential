#ifndef aclutil_literal_h
#define aclutil_literal_h

#include "./iddef.h"
#include "./log.h"

enum ACLUTIL_LITERAL_MASK_ {
	ACLUTIL_LITERAL_MASK_RT,
	ACLUTIL_LITERAL_MASK_CONSTANT	= 0x1,
	ACLUTIL_LITERAL_MASK_LITERAL	= 0x3
};

enum ACLUTIL_LITERAL_ {
	ACLUTIL_LITERAL_RT,
	ACLUTIL_LITERAL_CONSTANT,
	ACLUTIL_LITERAL_LITERAL
};

typedef aclspv_wrd_t	aclutil_ltrlmask_t;
typedef union {
	int		m_api_int;
	intmax_t	m_api_intmax;
	uintmax_t	m_api_uintmax;

	u64_least	m_u64;
	double		m_dbl;
	float		m_flt;
} aclutil_literal;

struct aclutil_ltrl_eval_t {
	aclutil_literal	m_ret;
	int		m_found;
};

ae2f_inline_f static ae2f_ccconst struct aclutil_ltrl_eval_t	aclutil_literal_eval2(
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

				default:
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
				RET.m_found = 0;
				return RET;
			}
			break;

		default:
			break;
	}

	RET.m_found = 0;

#undef	EXPR_TWO
	return RET;
}

#endif
