#ifndef	aclcur_lbl_pure_h
#define	aclcur_lbl_pure_h

#include "./id.h"

/**
 * register-level colouring of one label.
 *
 * a register is *defined in* a label when its ACLCUR_REG node was carved
 * inside that label's region (the node's m_lbl is the label's id). a
 * register carved in any other label — the root included — is external to
 * it.
 *
 * - const: the label READS (ACLCUR_LOAD) no register defined outside
 *   itself, so every value it consumes is produced by its own region.
 * - pure:  the label WRITES (ACLCUR_STORE) no register defined outside
 *   itself, so it leaves no effect observable after the block.
 *
 * ACLCUR_FLAG0_LBL_NOCONSTANT is the negation of const,
 * ACLCUR_FLAG0_LBL_NOPURE the negation of pure. the naming follows the
 * gcc function attributes: a pure block may read state it does not own
 * (only the read of external state makes it non-const), a const block
 * reads nothing external at all.
 *
 * only register-bound accesses are attributed. an ACLCUR_LOAD /
 * ACLCUR_STORE node names the register it was issued on (load and store
 * carving resolve aliases first, so the recorded id is the register
 * owning the access, or the view that is an access subject). value-flow
 * nodes carry value/address operands, not registers, and are not
 * counted: ACLCUR_OP*, ACLCUR_OPX and the result-less ACLCUR_ST whose
 * pointer operand is an already-computed value. an unresolvable register
 * counts as external (conservative).
 *
 * this is access LOCALITY, not data flow: every recorded access counts,
 * including a LOAD of an interface resource root (an address). the
 * function-level rule below filters that case.
 * */

/**
 * recompute and store c_lbl's ACLCUR_FLAG0_LBL_NOCONSTANT and
 * ACLCUR_FLAG0_LBL_NOPURE bits from its current region.
 *
 * a label's region is final only once it is sealed (the next label opens,
 * or the carve ends), so a caller snapshotting the bits does it then. the
 * predicates below recompute instead of trusting the stored bits.
 *
 * returns ACLSPV_COMPILE_OK, or ACLSPV_COMPILE_UNHANDLED when c_lbl is not
 * a label.
 * */
aclspv_compile_t aclcur_lbl_cfg_pure(aclcur_lblid_t c_lbl, h_aclspv_ctx_t h_ctx);

/** true when c_lbl reads no register defined outside it. recomputes the
 * bits first: the answer must not come from a stale snapshot. */
aclspv_bool_t aclcur_lbl_is_const(aclcur_lblid_t c_lbl, h_aclspv_ctx_t h_ctx);

/** true when c_lbl writes no register defined outside it. recomputes the
 * bits first: the answer must not come from a stale snapshot. */
aclspv_bool_t aclcur_lbl_is_pure(aclcur_lblid_t c_lbl, h_aclspv_ctx_t h_ctx);

/**
 * function-level purity of the function currently carved in h_ctx.
 *
 * the cursor arena holds exactly one function: aclemit_root resets
 * m_num_cursor per function declaration, so the whole buffer is the scope.
 * a register is *owned by the function* when its storage root carries
 * ACLCUR_FLAG0_REG_SCOPED — set at register creation for Function storage
 * only. an ALIAS2 view copies no policy, so the chain is walked to the
 * storage owner (as aclcur_reg_needs_memory does). every other storage
 * class (Private, Workgroup, Uniform, StorageBuffer, UniformConstant,
 * Input, Output, PushConstant, ...) is state the function does not own, and
 * so is a ghost (an anchor on a pointer value it does not own). an
 * unresolvable register counts as external.
 *
 * dead blocks are excluded: the emission sweep powers them off, so their
 * instructions never reach the binary. see aclcur_ssa_lbl_live().
 *
 * a LOAD of an external interface register that names no view is not a read
 * of state: it materializes the resource pointer (`out` / `in` yields the
 * address). reads that count go through a GHOST or an ALIAS2 view. a STORE
 * of any external register is a write.
 *
 * LIFETIME: the answer is a property of the carved graph, and aclemit resets
 * the cursor arena at FunctionEnd (see aclemit_fndecl_bodycfg's
 * h_ctx->m_num_cursor = 0). call these while the function is being carved:
 * the pipeline does it post-carve / pre-emit and persists the bits as
 * ACLSYM_FN_FLAG0_CONST / ACLSYM_FN_FLAG0_PURE — in aclsym_fn_t::m_flag0
 * for a normal function, or aclsym_entp_t::m_flag0 for an entry point
 * (which has no symbol: it is not a call target). afterwards the arena is
 * empty and the queries report 0 (unknown).
 *
 * note the boundary differs from the label-level rule on purpose. label
 * membership cannot answer this question: entry-point argument registers
 * are carved in the function's own root label, and a same-block view of an
 * external buffer would read as local. the storage class is the boundary.
 *
 * - const: no live block reads a register the function does not own.
 * - pure:  no live block writes a register the function does not own.
 * */

/** recompute and store the function-level bits on the root label. */
aclspv_compile_t aclcur_fn_cfg_pure(h_aclspv_ctx_t h_ctx);

/** true when the current function reads no register it does not own.
 * recomputes the bits first. */
aclspv_bool_t aclcur_fn_is_const(h_aclspv_ctx_t h_ctx);

/** true when the current function writes no register it does not own.
 * recomputes the bits first. */
aclspv_bool_t aclcur_fn_is_pure(h_aclspv_ctx_t h_ctx);

#endif
