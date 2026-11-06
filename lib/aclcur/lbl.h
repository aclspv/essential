#ifndef	aclcur_lbl_h
#define aclcur_lbl_h

#include <aclcur.h>
#include "./class.h"
#include "./flag0.h"
#include "./lbl/id.h"

enum ACLCUR_FLAG0_LBL_ {
	ACLCUR_FLAG0_LBL_ROOT	= ACLCUR_FLAG0_MAX,
	ACLCUR_FLAG0_LBL_LOOP,
	ACLCUR_FLAG0_LBL_MAX

};

aclcur_flag0_tent(ACLCUR_FLAG0_LBL_, aclcur_flag0_lbl);

typedef struct aclcur_lbl {
	/** PHYSICAL number of operations, INCLUDING SELF. */
	aclspv_wrd_t		m_nop;

	/** root of it. */
	aclcur_lblid_t		m_root;

	/** cursor-buffer index of the currently open label.
	 * only the root label's value is authoritative; see
	 * aclcur_get_idx_lbl_cur(). ACLCUR_IDX_INVAL when none. */
	aclcur_idx_t		m_idx_cur;

	/** function-level: the last traversed top-level path ended in a
	 * return. only the root label's value is authoritative. */
	aclspv_bool_t		m_fnret;

	/** current lexical loop targets. only the root label's values are
	 * authoritative; a while saves/sets/restores them around its body. */
	aclspv_id_t		m_break_loop;
	aclspv_id_t		m_continue_loop;
} aclcur_lbl_t;

/** 
 * get current branch label.
 * has a possibility to fail.
 * */
aclcur_lblid_t ae2f_ccpure aclcur_get_lbl_last(h_aclspv_ctx_t h_ctx);
aclcur_t* ae2f_ccpure aclcur_get_lbl_last1(h_aclspv_ctx_t h_ctx);

/** cursor-buffer index of the currently open label (the one the next node
 * belongs to). O(1): read from the root label. ACLCUR_IDX_INVAL before the
 * root is bootstrapped. */
aclcur_idx_t ae2f_ccpure aclcur_get_idx_lbl_cur(const x_aclspv_ctx* ae2f_restrict h_ctx);

/** the currently open label, or 0 before the root is bootstrapped. */
aclcur_t* ae2f_ccpure aclcur_get_lbl_cur1(h_aclspv_ctx_t h_ctx);

/** id of the currently open label, or 0. */
aclspv_id_t ae2f_ccpure aclcur_get_lbl_cur(h_aclspv_ctx_t h_ctx);

/** function-level "last top-level path returned" bit. */
aclspv_bool_t ae2f_ccpure aclcur_get_fnret(const x_aclspv_ctx* ae2f_restrict h_ctx);
void aclcur_set_fnret(aclspv_bool_t c_ret, h_aclspv_ctx_t h_ctx);

/** current lexical loop targets, 0 when not in a loop. */
aclspv_id_t ae2f_ccpure aclcur_get_break_loop(const x_aclspv_ctx* ae2f_restrict h_ctx);
aclspv_id_t ae2f_ccpure aclcur_get_continue_loop(const x_aclspv_ctx* ae2f_restrict h_ctx);
void aclcur_set_break_loop(aclspv_id_t c_lbl, h_aclspv_ctx_t h_ctx);
void aclcur_set_continue_loop(aclspv_id_t c_lbl, h_aclspv_ctx_t h_ctx);

/** root is not bootstrapped by this. */
aclcur_lblid_t aclcur_cfg_lbl(aclcur_lblid_t c, h_aclspv_ctx_t h_ctx);

/** 
 * returns 0 on failure
 *
 * mark a label as the loop header (the while-cond block).
 * only the header takes the marker: the back edge is not carved yet
 * when its loads resolve, so no graph shape reveals the loopness then.
 * everything else (the body, the store-side and phi-side decisions)
 * is derived from graph structure: the loop JC's continue target at
 * carve time, and emit-time cycle membership.
 *
 * */
aclcur_lblid_t aclcur_cfg_lbl_loop(aclcur_lblid_t c, h_aclspv_ctx_t h_ctx);

/**
 * return type of the current function: the root label's `m_ty`.
 *
 * the root label is node 0 of the cursor buffer (see aclcur_cfg_root);
 * this is the single source of truth for the return type. returns
 * ACLSPV_ID_VOID when the buffer has no root yet.
 * */
aclspv_id_t ae2f_ccpure aclcur_get_retty(const x_aclspv_ctx* ae2f_restrict h_ctx);

/** 
 * bootstrap the root label as the first node of the cursor buffer.
 *
 * unlike aclcur_cfg_lbl, this does not require a previous label:
 * it appends node 0 by hand (no get_lbl_last1 needed).
 *
 * c_retty becomes the node's m_ty (the function return type, see
 * struct aclcur::m_ty).
 *
 * the buffer must be empty when called.
 * */
aclspv_id_t aclcur_cfg_root(aclspv_id_t c_retty, h_aclspv_ctx_t h_ctx);

aclspv_compile_t EMIT_aclcur_lbl(h_aclcur_t h, h_aclspv_ctx_t h_ctx);

#include "./lbl/dfs.h"
#include "./lbl/reach.h"

#endif
