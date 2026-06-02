#ifndef aclexpr_op_h
#define aclexpr_op_h

#include <util/cxtp/vec.h>
#include <util/literal.h>

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
	ACLEXPR_OP1_EXPLICIT_CAST,
	ACLEXPR_OP1_POSTINC,
	ACLEXPR_OP1_PREINC,
	ACLEXPR_OP1_POSTDEG,
	ACLEXPR_OP1_PREDEG,
	ACLEXPR_OP2_ARITHMETIC_ADD,
	ACLEXPR_OP2_ARITHMETIC_SUB,
	ACLEXPR_OP2_ARITHMETIC_MUL,
	ACLEXPR_OP2_ARITHMETIC_REM,
	ACLEXPR_OP2_ARITHMETIC_DIV,
	ACLEXPR_OP2_ASSIGN
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
typedef aclspv_wrd_t aclexpr_opbin2_t;

#endif
