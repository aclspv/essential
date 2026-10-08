#ifndef	aclcur_common_h
#define aclcur_common_h

#include "../aclcur.h"
#include "./lbl.h"
#include "./j.h"
#include "./jc.h"
#include "./reg.h"
#include "./class.h"
#include "./load.h"
#include "./phi.h"
#include "./store.h"
#include "./op.h"
#include "./opX.h"
#include "./emit.h"

struct aclcur {
	union {
		aclcur_lbl_t	m_lbl;
		aclcur_j_t	m_j;
		aclcur_jc_t	m_jc;
		aclcur_reg_t	m_reg;
		aclcur_load_t	m_load;
		aclcur_store_t	m_store;
		aclcur_phi_t	m_phi;
		aclcur_op_t	m_op;
		aclcur_opx_t	m_opx;
		aclcur_opx_mbr_t	m_opx_mbr;
	}		m_d;

	CXCursor	m_cur;

	/** enum ACLCUR_FLAG0_ */
	aclspv_wrd_t	m_flag0;
	aclspv_id_t	m_id;

	/** current label */
	aclspv_id_t	m_lbl;

	/** enum ACLCUR_ */
	aclspv_wrd_t	m_class;
	aclspv_id_t	m_val;

	/** 
	 * type of m_val.
	 *
	 * special: for 0th element(root label), this is a return type of the function.
	 * */
	aclspv_id_t	m_ty;

	/** number of cursor referenced. */
	aclspv_wrd_t	m_num_ref;

	/** number of word count to be skipped to get next cursor. */
	aclspv_wrd_t	m_num_skip;
	/** number of word count to be skipped to get previous cursor */
	aclspv_wrd_t	m_num_skip_prv;

	/** depth of this cursor */
	aclspv_wrd_t	m_depth;

	/** a value tracked on current branch (label) */
	aclutil_literal m_val_lit;

	/** 
	 * literal mask for predicted value.
	 * see enum ACLUTIL_LITERAL_
	 * */
	aclspv_wrd_t	m_val_mask;
};

aclspv_id_t		aclcur_cfg_node2(aclspv_id_t c_newid, h_aclspv_ctx_t h_ctx, aclspv_bool_t c_toskip);

/** 
 * this assigns new id
 * assumes skipping is not happening
 *
 * see aclcur_cfg_node2
 * */
static ae2f_inline_f aclspv_id_t aclcur_cfg_node(aclspv_id_t c, h_aclspv_ctx_t h_ctx) {
	return aclcur_cfg_node2(c, h_ctx, 0);
}

ae2f_ccpure aclcur_t*	aclcur_get_node(aclspv_id_t c, const x_aclspv_ctx* ae2f_restrict h_ctx);
ae2f_ccpure aclcur_t* aclcur_get_node_prv(aclspv_id_t c, const x_aclspv_ctx* ae2f_restrict h_ctx);
ae2f_ccpure aclcur_t* aclcur_get_node_nxt(aclspv_id_t c, const x_aclspv_ctx* ae2f_restrict h_ctx);

/** 
 * sets the c's cursor to c_cur.
 * when c_cur is null cursor it changes none of c.
 *
 * returns
 * : null cursor `clang_getNullCursor` when no id was found.
 * : a cursor it have previously had.
 * */
CXCursor aclcur_set_cxcursor(aclspv_id_t c, const CXCursor c_cur, h_aclspv_ctx_t h_ctx);

#endif
