#ifndef	aclutil_op_h
#define aclutil_op_h

#include <aclspv.h>

/** 
 * returns whether the operation order matters
 * */
ae2f_inline_f static ae2f_ccconst aclspv_bool_t aclutil_op_ordered(const aclspv_wrd_t c) {
	switch(c) {
		/** rest */
		default:
			return 1;

		case SpvOpIAdd:
		case SpvOpIMul:
		case SpvOpFAdd:
		case SpvOpFMul:
		case SpvOpBitwiseOr:
		case SpvOpBitwiseAnd:
		case SpvOpBitwiseXor:
		case SpvOpLogicalEqual:
		case SpvOpLogicalNotEqual:
			return 0;
	}
}

#endif
