#ifndef	aclcur_store_h
#define aclcur_store_h

#include <aclspv.h>
#include "./flag0.h"

enum ACLCUR_FLAG0_STORE_ {
	/** store-to-memory is guaranteed */
	ACLCUR_FLAG0_STORE_EXPLICIT = ACLCUR_FLAG0_MAX,
	ACLCUR_FLAG0_STORE_MAX
};


aclcur_flag0_tent(ACLCUR_FLAG0_STORE_, aclcur_flag0_store);

typedef struct aclcur_store {
	aclspv_id_t	m_reg;
} aclcur_store_t;

/** returns a store id of `c_reg` from `c_lbl` */
ae2f_ccpure aclspv_id_t	aclcur_get_node_store(
		aclspv_id_t c_reg,
		aclspv_id_t c_lbl,
		h_aclspv_ctx_t h_ctx
		);

/**
 * normal store to memory
 * assumes branching is happening
 * 
 * phi is guaranteed
 * */
aclspv_id_t	aclcur_store_reg(
		aclspv_id_t c_reg,
		aclspv_id_t c_val,
		h_aclspv_ctx_t h_ctx
		);

/**
 * implicit store to memory
 * assumes no branching is happening
 * */
aclspv_id_t	aclcur_store_reg_implicit(
		aclspv_id_t c_reg,
		aclspv_id_t c_val,
		h_aclspv_ctx_t h_ctx
		);

/** 
 * explicit store to memory
 * phi/store-to-memory is guaranteed.
 * */
aclspv_id_t	aclcur_store_reg_explicit(
		aclspv_id_t c_reg,
		aclspv_id_t c_val,
		h_aclspv_ctx_t h_ctx
		);

#endif
