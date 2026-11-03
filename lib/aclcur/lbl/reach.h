#ifndef	aclcur_lbl_reach_h
#define aclcur_lbl_reach_h

#include "./id.h"

aclspv_bool_t aclcur_lbl_is_reachable(
		aclcur_lblid_t c_root,
		aclcur_lblid_t c_child,
		h_aclspv_ctx_t h_ctx
		);

/** unconditionally reachable */
aclspv_bool_t aclcur_lbl_is_reachable_uc(
		aclcur_lblid_t c_root,
		aclcur_lblid_t c_child,
		h_aclspv_ctx_t h_ctx);

/** true iff a successor of c_lbl can reach c_lbl back: the block
 * participates in a cycle (a loop block: header, body, or the continue
 * trampoline). */
aclspv_bool_t aclcur_lbl_in_cycle(
		aclcur_lblid_t c_lbl,
		h_aclspv_ctx_t h_ctx);

#endif
