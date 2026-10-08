#ifndef	aclcur_lbl_h
#define aclcur_lbl_h

#include <aclspv.h>
#include "./class.h"
#include "./flag0.h"

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
	aclspv_id_t		m_root;
} aclcur_lbl_t;

/** 
 * get current branch label.
 * has a possibility to fail.
 * */
aclspv_id_t ae2f_ccpure aclcur_get_lbl_last(h_aclspv_ctx_t h_ctx);
aclcur_t* ae2f_ccpure aclcur_get_lbl_last1(h_aclspv_ctx_t h_ctx);

/** root is not bootstrapped by this. */
aclspv_id_t aclcur_cfg_lbl(aclspv_id_t c, h_aclspv_ctx_t h_ctx);

/** mark a label as the loop header (the while-cond block).
 * only the header takes the marker: the back edge is not carved yet
 * when its loads resolve, so no graph shape reveals the loopness then.
 * everything else (the body, the store-side and phi-side decisions)
 * is derived from graph structure: the loop JC's continue target at
 * carve time, and emit-time cycle membership. */
aclspv_id_t aclcur_cfg_lbl_loop(aclspv_id_t c_lbl, h_aclspv_ctx_t h_ctx);

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
