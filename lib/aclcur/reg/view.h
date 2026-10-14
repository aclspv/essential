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
		aclspv_wrd_t		c_step,
		h_aclspv_ctx_t		h_ctx
		);

/**
 * create (or reuse) a GHOST register anchored on the pointer VALUE c_base
 * (D6): a parentless DEREF view. its m_ty is the pointee of c_base's
 * pointer type, its storage class that of the pointer; its m_base is the
 * held pointer value, and its own m_id is a fresh compile-time handle.
 * no storage is emitted (m_emit_reg == 0, like a view); all address work is
 * forwarded through m_base (see aclcur_reg_base).
 *
 * dedup key (P3 open item): (m_base, m_ty) — a held pointer value plus a
 * pointee type names one region, so two member accesses through the same
 * pointer share one ghost (and, with aclcur_reg_view's dedup, one view).
 *
 * the held pointer value is kept referenced (an unreferenced op is elided
 * by the emit pass, and the ghost's chain names it later).
 *
 * @return the ghost register id, or 0 on failure (c_base is not a pointer).
 * */
aclspv_id_t aclcur_reg_ghost(aclspv_id_t c_base, h_aclspv_ctx_t h_ctx);

/**
 * stock "view" register interface: parent-relative access.
 *
 * - m_addr: aclcur_regview_addr, the emit-time OpAccessChain node append
 *   (see its contract in reg.h, ACLCUR_FLAG0_REG_ALIAS2);
 * - m_size: aclcur_regview_size, the byte size of the view's own type;
 * - no m_emit_reg: a view owns no storage of its own, it is
 *   materialized lazily by its m_addr.
 *
 * - m_emit_load / m_emit_store (and their explicit variants): the
 *   emission hooks a LOAD / STORE node bound to this view dispatches to.
 *   they mint the view's OpAccessChain as RAW WORDS immediately before
 *   the OpLoad / OpStore, mirroring the push-constant leaf hooks
 *   (aclemit/fndecl/entp/args.c): minting an op-node at emit time would
 *   land at the buffer tail and emit after its own use.
 * */
aclspv_compile_t aclcur_regview_emit_load(aclspv_id_t c_id, h_aclspv_ctx_t h_ctx);
aclspv_compile_t aclcur_regview_emit_load_explicit(aclspv_id_t c_id, h_aclspv_ctx_t h_ctx);
aclspv_compile_t aclcur_regview_emit_store(aclspv_id_t c_id, h_aclspv_ctx_t h_ctx);
aclspv_compile_t aclcur_regview_emit_store_explicit(aclspv_id_t c_id, h_aclspv_ctx_t h_ctx);

/**
 * stock "ghost" register interface: a chain root anchored on a held pointer
 * value, not on any storage the register owns.
 *
 * - m_addr: aclcur_regghost_addr, the held pointer value (m_base) by
 *   identity — `&*p` is `p`; side-effect free, idempotent;
 * - m_size: aclcur_regview_size, the byte size of the pointee type;
 * - no m_emit_reg: a ghost owns no storage, nothing is allocated;
 * - m_emit_load / m_emit_store (and their explicit variants): emit the
 *   OpLoad / OpStore with m_base as the memory operand.
 * */
aclspv_id_t aclcur_regghost_addr(aclspv_id_t c_reg, h_aclspv_ctx_t h_ctx);
aclspv_compile_t aclcur_regghost_emit_load(aclspv_id_t c_id, h_aclspv_ctx_t h_ctx);
aclspv_compile_t aclcur_regghost_emit_load_explicit(aclspv_id_t c_id, h_aclspv_ctx_t h_ctx);
aclspv_compile_t aclcur_regghost_emit_store(aclspv_id_t c_id, h_aclspv_ctx_t h_ctx);
aclspv_compile_t aclcur_regghost_emit_store_explicit(aclspv_id_t c_id, h_aclspv_ctx_t h_ctx);

ae2f_inline_f static ae2f_ccconst aclcur_reg_t aclcur_regghost_in(void) {
	const aclcur_reg_t in = {
		{ 0, 0, aclcur_regghost_addr, aclcur_regview_size, 0, aclcur_regghost_emit_load, aclcur_regghost_emit_load_explicit, aclcur_regghost_emit_store, aclcur_regghost_emit_store_explicit },
		{ { 0 } },
		0, 0, 0, 0, 0, 0, 0
	};
	return in;
}

ae2f_inline_f static ae2f_ccconst aclcur_reg_t aclcur_regview_in(void) {
	const aclcur_reg_t in = {
		{ 0, 0, aclcur_regview_addr, aclcur_regview_size, 0, aclcur_regview_emit_load, aclcur_regview_emit_load_explicit, aclcur_regview_emit_store, aclcur_regview_emit_store_explicit },
		{ { 0 } },
		0, 0, 0, 0, 0, 0, 0
	};
	return in;
}

#endif
