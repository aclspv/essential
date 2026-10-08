#ifndef aclcur_opX_h
#define aclcur_opX_h

#include <aclspv.h>

/** @brief max operands an opX node accepts (fixed-size staging array in cfg_opx). */
#define ACLCUR_OPX_OPRND_CAP 256

/** @brief operand-link node for >3-operand (opX) ops. chained via cursor-buffer index, like aclty mbr nodes. */
typedef struct {
	aclspv_id_t	m_operand;
	/** @brief next link node's cursor-buffer index. 0 = terminator. */
	aclspv_wrd_t	m_nxt;
} aclcur_opx_mbr_t;

/** @brief op with variable operand count. operands form a linked list of aclcur_opx_mbr_t. */
typedef struct {
	enum SpvOp_	m_op;
	/** @brief cursor-buffer index of the first operand-link node. 0 = none. */
	aclspv_wrd_t	m_o_head;
	/** @brief operand count. */
	aclspv_wrd_t	m_num;
} aclcur_opx_t;

aclspv_id_t aclcur_cfg_opx(
		aclspv_id_t	c_newid_opt,
		enum SpvOp_	c_op,
		aclspv_id_t	c_ty,
		const aclspv_id_t	c_os[],
		aclspv_wrd_t	c_num,
		h_aclspv_ctx_t	h_ctx
		);

#include "./opX/emit.h"

#endif

