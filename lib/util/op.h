#ifndef util_op_h
#define util_op_h

#include <util/cxtp/vec.h>

enum UTIL_OPBINCFG_ {
	UTIL_OPBINCFG_NUMOPRND,

	/** @see ./literal.h */
	UTIL_OPBINCFG_MASKLITERAL,
	UTIL_OPBINCFG_SPECCONSTANTOP,
	UTIL_OPBINCFG_RESTYID,
	UTIL_OPBINCFG_RESID,
	UTIL_OPBINCFG_OPCODE,
	UTIL_OPBINCFG_END
};

enum UTIL_OPBIN1_ {
	UTIL_OPBIN1_NONE,
	UTIL_OPBIN1_EXPLICIT_CAST,
	UTIL_OPBIN1_END
};

enum UTIL_OPBIN2_ {
	UTIL_OPBIN2_NONE,
	UTIL_OPBIN2_ADD,
	UTIL_OPBIN2_SUB,
	UTIL_OPBIN2_MUL,
	UTIL_OPBIN2_REM,
	UTIL_OPBIN2_DIV,
	UTIL_OPBIN2_ASSIGN
};

#include <ae2f/Keys.h>
#include <ae2f/c90/StdInt.h>

#include "./iddef.h"
#include "./literal.h"
#include <spirv/unified1/spirv.h>

typedef struct {
	aclspv_wrd_t	m_type_original;
	aclspv_wrd_t	m_mask_literal;
	util_literal	m_literal;
	CXCursor	m_cursor;

	aclspv_bool_t	m_is_assign;
} util_opbincfg_extra;

/**
 * @brief type alias for `enum UTIL_OPBIN2_`
 * */
typedef uint_leastenum_t B_util_opbin2_t;


ae2f_unused ae2f_inline_f static ae2f_ccconst aclspv_bool_t	util_opbin2_is_assign(
		const enum UTIL_OPBIN2_ c_op2
		)  {
	return c_op2 == UTIL_OPBIN2_ASSIGN;
}

ae2f_unused ae2f_inline_f static B_util_opbin2_t util_opbin2_to_spvop(
		const e_id_default c_id, 
		const enum UTIL_OPBIN2_ c_op2,
		const h_aclspv_ctx_t	h_ctx
		)
{
	aclspv_id_t	ID = aclid_is_vectorisable((aclspv_id_t) c_id) ? (aclspv_id_t) c_id
		: aclutil_cxtp_get_veccfg((aclspv_id_t) c_id, h_ctx).m_ty_elm;

	switch(c_op2) {
		case UTIL_OPBIN2_NONE:
		case UTIL_OPBIN2_ASSIGN:
		default:
			return 0;

		case UTIL_OPBIN2_ADD:
			if(aclid_is_int(ID))
				return SpvOpIAdd;
			else if(aclid_is_float(ID))
				return SpvOpFAdd;
			else return 0;

		case UTIL_OPBIN2_DIV:
			if(aclid_is_signed(ID))
				return SpvOpSDiv;
			else if (aclid_is_unsigned(ID))
				return SpvOpUDiv;
			else if(aclid_is_float(ID))
				return SpvOpFDiv;
			else return 0;

		case UTIL_OPBIN2_MUL:
			if(aclid_is_signed(ID))
				return SpvOpIMul;
			else if (aclid_is_unsigned(ID))
				return SpvOpIMul;
			else if(aclid_is_float(ID))
				return SpvOpFMul;
			else return 0;

		case UTIL_OPBIN2_REM:
			if(aclid_is_signed(ID))
				return SpvOpSRem;
			else if (aclid_is_unsigned(ID))
				return SpvOpUMod;
			else if(aclid_is_float(ID))
				return SpvOpFMod;
			else return 0;

		case UTIL_OPBIN2_SUB:
			if(aclid_is_signed(ID))
				return SpvOpISub;
			else if (aclid_is_unsigned(ID))
				return SpvOpISub;
			else if(aclid_is_float(ID))
				return SpvOpFSub;
			else return 0;
	}
}

#endif
