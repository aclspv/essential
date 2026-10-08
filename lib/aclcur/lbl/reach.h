#ifndef	aclcur_lbl_reach_h
#define aclcur_lbl_reach_h

#include <aclspv.h>

aclspv_bool_t aclcur_lbl_is_reachable(
		aclspv_id_t c_root,
		aclspv_id_t c_child,
		h_aclspv_ctx_t h_ctx
		);

/** unconditionally reachable */
aclspv_bool_t aclcur_lbl_is_reachable_uc(
		aclspv_id_t c_root,
		aclspv_id_t c_child,
		h_aclspv_ctx_t h_ctx);

/** true iff a successor of c_lbl can reach c_lbl back: the block
 * participates in a cycle (a loop block: header, body, or the continue
 * trampoline). */
aclspv_bool_t aclcur_lbl_in_cycle(
		aclspv_id_t c_lbl,
		h_aclspv_ctx_t h_ctx);

#endif
