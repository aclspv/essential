#include "./align.h"
#include <.aclspv/const/spec.h>


#define ADD(r, c_x, c_y) {	\
	(r).m_id = 0;		\
	(r).m_lit.m_wrd = (c_x).m_lit.m_wrd + (c_y).m_lit.m_wrd;	\
}

#define SUB(r, c_x, c_y) {	\
	(r).m_id = 0;		\
	(r).m_lit.m_wrd = (c_x).m_lit.m_wrd - (c_y).m_lit.m_wrd;	\
}

#define RSHFT(r, c_x, c_y) {	\
	(r).m_id = 0;		\
	(r).m_lit.m_wrd = (c_x).m_lit.m_wrd >> (c_y).m_lit.m_wrd;	\
}

#define OR(r, c_x, c_y) {	\
	(r).m_id = 0;		\
	(r).m_lit.m_wrd = (c_x).m_lit.m_wrd | (c_y).m_lit.m_wrd;	\
}

aclconst_specsz	aclconst_align_from_size(
		aclconst_specsz	c,
		h_aclspv_ctx_t	h_ctx
		) {
	aclconst_specsz op0, op1, res;
	aclspv_wrd_t top = h_ctx->m_usrcfg.m_byte_size_type << 3, b;

	op1.m_id	= 0;
	op1.m_lit.m_wrd	= 1;
	_aclconst_specsize_op2(L, res, c, op1, SpvOpISub, SUB, h_ctx);

	for(b = 1; b < top; b <<= 1) {
		op0 = res;
		op1.m_id	= 0;
		op1.m_lit.m_wrd	= b;
		_aclconst_specsize_op2(L, res, op0, op1, SpvOpShiftRightLogical, RSHFT, h_ctx);

		op1 = res;
		_aclconst_specsize_op2(L, res, op0, op1, SpvOpBitwiseOr, OR, h_ctx);
	}

	op0 = res;
	op1.m_id	= 0;
	op1.m_lit.m_wrd	= 1;
	_aclconst_specsize_op2(L, res, op0, op1, SpvOpIAdd, ADD, h_ctx);
	return res;
}

aclconst_specsz	aclconst_align_from_size2(
		aclspv_wrd_t	c,
		h_aclspv_ctx_t	h_ctx
		) {
	aclconst_specsz	co;
	co.m_id		= 0;
	co.m_lit.m_wrd	= c;

	return aclconst_align_from_size(co, h_ctx);
}
