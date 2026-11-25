#ifndef	aclcur_op_none_h
#define	aclcur_op_none_h

#include <aclcur.h>
#include <aclctx/id.h>

#include <aclspv.h>

/**
 * construction of the instructions that carry NO Result Type and NO Result:
 *
 *   SpvOpControlBarrier  [op|4][execution scope][memory scope][semantics]
 *   SpvOpMemoryBarrier   [op|3][memory scope][semantics]
 *
 * this is the ACLCUR_OPX shape (a linked operand chain, so the words are
 * appended in program order and emitted later like every other instruction)
 * with a zero type: EMIT_aclcur_opx writes a type and a result id only for a
 * value node, and these instructions have neither.
 *
 * the words are scope / memory-semantics values, not ids. OpControlBarrier and
 * OpMemoryBarrier still take them as id operands, so `aclcur_cfg_barrier`
 * folds each one into an OpConstant of the builtin's own u32 parameter type.
 * */

/** @return the new node id, or 0 on failure. */
aclcur_id_t aclcur_cfg_barrier(
		enum SpvOp_	c_op,
		const aclspv_wrd_t	c_words[],
		aclspv_wrd_t		c_num,
		h_aclspv_ctx_t		h_ctx
		);

#endif
