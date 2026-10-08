#ifndef	aclcur_ssa_h
#define	aclcur_ssa_h

#include <aclspv.h>

/** emit-liveness of a block, shared between the value-flow resolution
 * (load.c) and the variable elision (reg/var.c).
 *
 * the emission sweep powers off every label whose reference count
 * cascades to zero; a block whose instructions never land in the binary
 * cannot contribute stores, exit values, or references. the predicate
 * follows the predecessor edges up to the root (see load.c). */
aclspv_bool_t aclcur_ssa_lbl_live(aclspv_id_t c_lbl, h_aclspv_ctx_t h_ctx);

/** post-carve, pre-emit: complete the loop-header authoritative phi
 * chains (ACLCUR_FLAG0_PHI_AUTH) with the final-graph back-edge
 * carries (see load.c). runs between carve and emission. */
aclspv_compile_t aclcur_ssa_loop_carry_fill(h_aclspv_ctx_t h_ctx);

#endif

