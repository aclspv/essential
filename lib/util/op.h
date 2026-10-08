#ifndef util_op_h
#define util_op_h

#include <util/cxtp/vec.h>
#include "./literal.h"

enum ACLEXPR_OPBINCFG_ {
	ACLEXPR_OPBINCFG_NUMOPRND,

	/** @see ./literal.h */
	ACLEXPR_OPBINCFG_MASKLITERAL,
	ACLEXPR_OPBINCFG_SPECCONSTANTOP,
	ACLEXPR_OPBINCFG_RESTYID,
	ACLEXPR_OPBINCFG_RESID,
	ACLEXPR_OPBINCFG_OPCODE,
	ACLEXPR_OPBINCFG_END
};

enum ACLEXPR_OP_ {
	UTIL_OPBIN1_NONE,
	ACLEXPR_OP1_EXPLICIT_CAST,
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

typedef struct {
	aclspv_wrd_t	m_type_original;
	aclspv_wrd_t	m_mask_literal;
	aclutil_literal	m_literal;
	CXCursor	m_cursor;

	aclspv_bool_t	m_is_assign;
} aclexpr_opbincfg_extra;

/**
 * @brief type alias for `enum UTIL_OPBIN2_`
 * */
typedef uint_leastenum_t aclexpr_opbin2_t;

#endif
