#ifndef aclexpr_op_h
#define aclexpr_op_h

#include <util/cxtp/vec.h>
#include <util/literal.h>

enum ACLEXPR_OPBINCFG_ {
	ACLEXPR_OPBINCFG_NUMOPRND,

	/** @see ./literal.h */

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
	ACLEXPR_OP1_POSTDEC,
	ACLEXPR_OP1_PREDEC,
	ACLEXPR_OP2_ARITHMETIC_ADD,
	ACLEXPR_OP2_ARITHMETIC_SUB,
	ACLEXPR_OP2_ARITHMETIC_MUL,
	ACLEXPR_OP2_ARITHMETIC_REM,
	ACLEXPR_OP2_ARITHMETIC_DIV,
	ACLEXPR_OP2_ASSIGN,
	ACLEXPR_OP2_AND,
	ACLEXPR_OP2_OR,
	ACLEXPR_OP2_XOR
};

typedef struct {
	aclspv_wrd_t	m_type_original;
	aclspv_wrd_t	m_mask_literal;
	aclutil_literal	m_literal;
	CXCursor	m_cursor;

	aclspv_wrd_t	m_state0;
} aclexpr_opbincfg_extra;

enum ACLEXPR_OPCFGX_STATE0_ {
	ACLEXPR_OPCFGX_STATE0_ASSIGN		= 0x1,
	ACLEXPR_OPCFGX_STATE0_VAL_NO_NEED	= 0x2,
	ACLEXPR_OPCFGX_STATE0_LOGICAL		= 0x4
};

/**
 * @brief type alias for `enum UTIL_OPBIN2_`
 * */
typedef aclspv_wrd_t aclexpr_opbin2_t;

#endif
