#ifndef aclcur_reg_view_h
#define aclcur_reg_view_h

#include "../reg.h"
#include <ae2f/cc/profiles.h>

/**
 * address of an ALIAS2 view register: EMIT-TIME, APPENDING,
 * NON-IDEMPOTENT.
 *
 * each call appends ONE OpAccessChain to the current function body
 * (m_section.m_fnimpl) and mints a new result id (h_ctx->m_id++):
 * result type is the view's pointer type, base is the ROOT register's
 * id (the var's pure-identity m_addr — the OpVariable /
 * OpFunctionParameter result id), then the element/member index ids,
 * root-most first, this view's own last.
 *
 * NEVER cache the result or call it twice for a single use: the second
 * call is a second opcode, and re-using one id across a dominating
 * re-materialization would violate dominance.
 * */
aclspv_id_t aclcur_regview_addr(aclspv_id_t c_reg, h_aclspv_ctx_t h_ctx);

/** byte size of an ALIAS2 view register's storage: the size of its own type. */
aclconst_specsz aclcur_regview_size(aclspv_id_t c_reg, h_aclspv_ctx_t h_ctx);

/**
 * create an ALIAS2 typed view register of the storage owned by c_parent.
 *
 * allocates one register node (skipped cursor, see aclcur_cfg_node2),
 * links it to c_parent via aclcur_reg_alias2() — which also marks the
 * whole ancestor chain kept — and stores c_idx as the single
 * element/member index the view adds to its parent (m_id_idx).
 *
 * no SPIR-V is emitted at factory time: the node materializes lazily,
 * one OpAccessChain per address request (see aclcur_regview_addr).
 *
 * returns the new view register id, or 0 on failure.
 * */
aclspv_id_t aclcur_reg_view(
		CXCursor		c_cur,
		aclspv_id_t		c_parent,
		aclspv_id_t		c_ty,
		aclspv_id_t		c_idx,
		aclspv_wrd_t		c_storage_class,
		h_aclspv_ctx_t		h_ctx
		);

/**
 * stock "view" register interface: parent-relative access.
 *
 * - m_addr: aclcur_regview_addr, the emit-time OpAccessChain append
 *   (see its contract in reg.h, ACLCUR_FLAG0_REG_ALIAS2);
 * - m_size: aclcur_regview_size, the byte size of the view's own type;
 * - no m_emit_reg: a view owns no storage of its own, it is
 *   materialized lazily by its m_addr.
 * */
ae2f_inline_f static ae2f_ccconst aclcur_reg_t aclcur_regview_in(void) {
	const aclcur_reg_t in = {
		{ 0, 0, aclcur_regview_addr, aclcur_regview_size, 0, 0, 0, 0, 0 },
		{ { 0 } },
		0, 0, 0, 0, 0
	};
	return in;
}

#endif
