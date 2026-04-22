#ifndef util_op_h
#define util_op_h

#include "util/id.h"
#include <clang-c/Index.h>
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
	UTIL_OPBIN1_EXPLICIT_CAST
};

enum UTIL_OPBIN2_ {
	UTIL_OPBIN2_NONE,
	UTIL_OPBIN2_ADD,
	UTIL_OPBIN2_SUB,
	UTIL_OPBIN2_MUL,
	UTIL_OPBIN2_REM,
	UTIL_OPBIN2_DIV,
	UTIL_OPBIN2_ADDASSIGN,
	UTIL_OPBIN2_SUBASSIGN,
	UTIL_OPBIN2_MULASSIGN,
	UTIL_OPBIN2_REMASSIGN,
	UTIL_OPBIN2_DIVASSIGN,
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
} util_opbincfg_extra;

/**
 * @brief type alias for `enum UTIL_OPBIN2_`
 * */
typedef uint_leastenum_t B_util_opbin2_t;


ae2f_unused ae2f_inline static ae2f_ccconst aclspv_bool_t	util_opbin2_is_assign(
		const enum UTIL_OPBIN2_ c_op2
		)  {
	switch((uint_fastenum_t)c_op2) {
		case UTIL_OPBIN2_ADDASSIGN:
		case UTIL_OPBIN2_SUBASSIGN:
		case UTIL_OPBIN2_DIVASSIGN:
		case UTIL_OPBIN2_REMASSIGN:
		case UTIL_OPBIN2_MULASSIGN:
		case UTIL_OPBIN2_ASSIGN:
			return 1;

		default:
			return 0;
	}
}

ae2f_unused ae2f_inline static ae2f_ccconst B_util_opbin2_t util_opbin2_to_spvop(
		const e_id_default c_id, 
		const enum UTIL_OPBIN2_ c_op2
		) 
{
	switch(c_op2) {
		case UTIL_OPBIN2_NONE:
		case UTIL_OPBIN2_ASSIGN:
		default:
			return 0;

		case UTIL_OPBIN2_ADD:
		case UTIL_OPBIN2_ADDASSIGN:
			if(aclid_is_int(c_id))
				return SpvOpIAdd;
			else if(aclid_is_float(c_id))
				return SpvOpFAdd;
			else return 0;

		case UTIL_OPBIN2_DIV:
		case UTIL_OPBIN2_DIVASSIGN:
			if(aclid_is_signed(c_id))
				return SpvOpSDiv;
			else if (aclid_is_unsigned(c_id))
				return SpvOpUDiv;
			else if(aclid_is_float(c_id))
				return SpvOpFDiv;
			else return 0;

		case UTIL_OPBIN2_MUL:
		case UTIL_OPBIN2_MULASSIGN:
			if(aclid_is_signed(c_id))
				return SpvOpIMul;
			else if (aclid_is_unsigned(c_id))
				return SpvOpIMul;
			else if(aclid_is_float(c_id))
				return SpvOpFMul;
			else return 0;

		case UTIL_OPBIN2_REM:
		case UTIL_OPBIN2_REMASSIGN:
			if(aclid_is_signed(c_id))
				return SpvOpSRem;
			else if (aclid_is_unsigned(c_id))
				return SpvOpUMod;
			else if(aclid_is_float(c_id))
				return SpvOpFMod;
			else return 0;

		case UTIL_OPBIN2_SUB:
		case UTIL_OPBIN2_SUBASSIGN:
			if(aclid_is_signed(c_id))
				return SpvOpISub;
			else if (aclid_is_unsigned(c_id))
				return SpvOpISub;
			else if(aclid_is_float(c_id))
				return SpvOpFSub;
			else return 0;
	}
}

#endif
