#ifndef	aclcur_op1_h
#define aclcur_op1_h

#include <aclcur.h>
#include <aclctx/id.h>
#include <aclty.h>

typedef struct {
	enum SpvOp_	m_op;
	aclspv_id_t	m_o[3];
} aclcur_op_t;

aclspv_vid_t aclcur_cfg_op1(
		aclspv_id_reserved_t c_newid_opt,
		enum SpvOp_ c_op,
		aclty_id_t c_ty,
		aclcur_vid_t c_o0,
		h_aclspv_ctx_t	h_ctx
		);

/** one-operand instruction whose operand is a RAW id rather than a value
 * node: the operand is emitted verbatim and is neither validated by
 * aclcur_get_load nor refcounted.
 *
 * an address is exactly this case — a register's id IS its storage address
 * and a register carries no ACLCUR_FLAG0_VALUE, so it can never pass
 * aclcur_cfg_op1's operand check. address-of and the view chain need it.
 * @see aclcur_cfg_op1 for the validated, refcounted form.
 * */
/** one-operand instruction whose operand is a RAW id rather than a value
 * node: [op|5][ty][id][raw_o0][imm].
 *
 * OpAccessChain's shape, whose Base is a register id. @see aclcur_cfg_op1r
 * */
aclspv_id_t aclcur_cfg_op1ri(
		aclspv_id_reserved_t c_newid_opt,
		enum SpvOp_ c_op,
		aclty_id_t c_ty,
		aclspv_vid_t c_o0,
		aclspv_wrd_t c_imm,
		h_aclspv_ctx_t	h_ctx
		);

/**
 * one-operand instruction whose operand is a RAW id, not a value node.
 *
 * the normal aclcur_cfg_op1 routes its operand through aclcur_get_load,
 * which requires a node carrying ACLCUR_FLAG0_VALUE. that is right for an
 * SSA operand, but an ADDRESS has no such node: a register's id is its
 * storage address, and a register deliberately carries no VALUE flag.
 * address-of (`&x`) and the view chain both land here — the address is
 * emitted verbatim, exactly as a constant operand would be.
 *
 * the raw operand takes no reference: it is backed by no node.
 *
 * @see aclcur_cfg_op1, whose operand is validated and refcounted.
 * */
aclspv_id_t aclcur_cfg_op1r(
		aclspv_id_reserved_t c_newid_opt,
		enum SpvOp_ c_op,
		aclty_id_t c_ty,
		aclspv_vid_t c_o0,
		h_aclspv_ctx_t	h_ctx
		);

aclspv_id_t aclcur_cfg_op2(
		aclspv_id_t c_newid_opt,
		enum SpvOp_ c_op,
		aclty_id_t c_ty,
		aclcur_vid_t c_o0,
		aclcur_vid_t c_o1,
		h_aclspv_ctx_t	h_ctx
		);

aclspv_id_t aclcur_cfg_op3(
		aclspv_id_t c_newid_opt,
		enum SpvOp_ c_op,
		aclspv_id_t c_ty,
		aclspv_id_t c_o0,
		aclspv_id_t c_o1,
		aclspv_id_t c_o2,
		h_aclspv_ctx_t	h_ctx
		);

/** one value operand plus an immediate literal word: same 5-word emission
 * shape as an op2 (SpvOpCompositeExtract, SpvOpAccessChain-style indexes).
 * @warning c_imm is a literal, not an id — it is emitted verbatim and must
 * not be referenced/refcounted. */
aclspv_id_t aclcur_cfg_op1i(
		aclspv_id_t c_newid_opt,
		enum SpvOp_ c_op,
		aclspv_id_t c_ty,
		aclspv_id_t c_o0,
		aclspv_wrd_t c_imm,
		h_aclspv_ctx_t	h_ctx
		);

/** two value operands plus an immediate literal word: same 6-word
 * emission shape as an op3 (SpvOpCompositeInsert: object, composite,
 * index).
 * @warning c_imm is a literal, not an id — it is emitted verbatim and
 * must not be referenced/refcounted. */
aclspv_id_t aclcur_cfg_op2i(
		aclspv_id_t c_newid_opt,
		enum SpvOp_ c_op,
		aclspv_id_t c_ty,
		aclspv_id_t c_o0,
		aclspv_id_t c_o1,
		aclspv_wrd_t c_imm,
		h_aclspv_ctx_t	h_ctx
		);

/** result-less two-operand instruction (SpvOpStore): [op|3][o0][o1].
 *
 * SpvOpStore carries neither Result Type nor Result, so none of the opN
 * shapes fit — their emitters always write a type and a result id. the node
 * is emitted in the emit pass like every other instruction, so it lands
 * after the instructions its operands came from; appending words during
 * evaluation instead would order it before them.
 *
 * the operands are referenced, so the emit pass does not drop the
 * instruction either of them came from.
 *
 * @return the new node id, or 0 on failure.
 * */
aclcur_id_t aclcur_cfg_st(
		enum SpvOp_	c_op,
		aclspv_id_t	c_o0,
		aclspv_id_t	c_o1,
		h_aclspv_ctx_t	h_ctx
		);

#include "./op/emit.h"

#endif
