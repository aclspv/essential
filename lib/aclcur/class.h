#ifndef	aclcur_class_h
#define aclcur_class_h

#include <aclspv.h>

enum ACLCUR_ {
	ACLCUR_RESERVED,
	/** SpvOpBranch */
	ACLCUR_J,
	/** label */
	ACLCUR_LBL,
	/** SpvOpBranchConditional */
	ACLCUR_JC,
	/** literal register. */
	ACLCUR_REG,
	ACLCUR_RET,
	/** load unpredicted register */
	ACLCUR_LOAD,
	ACLCUR_STORE,
	/** load from other branch */
	ACLCUR_PHI,
	ACLCUR_OP1,
	ACLCUR_OP2,
	ACLCUR_OP3,
	/** op with variable operand count (linked-list operand chain) */
	ACLCUR_OPX,
	/** operand-link node of an ACLCUR_OPX op. never emitted. */
	ACLCUR_OPX_MBR,
	/** result-less two-operand instruction (SpvOpStore): [op|3][o0][o1].
	 * no Result Type, no Result. emitted unconditionally — a store has
	 * side effects, so it is never dropped as unreferenced. */
	ACLCUR_ST
};

typedef struct aclcur	aclcur_t;
typedef struct aclcur* ae2f_restrict h_aclcur_t;

#endif
