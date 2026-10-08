/** it is written as load, but is actually value. */

#ifndef	aclcur_load_h
#define aclcur_load_h

#include <aclspv.h>

#include "./flag0.h"

#include <util/literal.h>

typedef struct aclcur_load {
	aclspv_id_t	m_reg;
	aclspv_wrd_t	m_num_phi;
	aclspv_id_t	m_phi;
} aclcur_load_t;


enum ACLCUR_FLAG0_LOAD_ {
	/** should be explicitly loaded from memory. */
	ACLCUR_FLAG0_LOAD_EXPLICIT = ACLCUR_FLAG0_MAX,
	ACLCUR_FLAG0_LOAD_MAX
};

aclcur_flag0_tent(ACLCUR_FLAG0_LOAD_, aclcur_flag0_load);

/** 
 * return value is always valid when non-zero.
 *
 * even when id is non-zero, a value could be 
 * use aclcur_reg_literalmask to get if r_lit_opt is valid.
 * */
aclspv_id_t	aclcur_load_reg(
		aclspv_id_t c_reg,
		h_aclspv_ctx_t h_ctx,
		aclutil_literal* ae2f_restrict r_lit_opt
		);

/**
 * explicit load: always reads memory (Volatile emission), never served
 * from the tracked-value cache. the LOAD node carries
 * ACLCUR_FLAG0_LOAD_EXPLICIT and dispatches to m_emit_load_explicit.
 * */
aclspv_id_t	aclcur_load_reg_explicit(
		aclspv_id_t c_reg,
		h_aclspv_ctx_t h_ctx,
		aclutil_literal* ae2f_restrict r_lit_opt
		);

#endif
