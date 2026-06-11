#ifndef	aclcursor_vardecl_h
#define aclcursor_vardecl_h

#include "./def.h"

#include <util/wrd.h>
#include <util/literal.h>

/** when cursor kind is arithmetic variable declaration */
typedef struct {
	aclutil_literal	m_literal;
	aclspv_wrd_t	m_stage_lvl;

	/**
	 * @var	m_id
	 *
	 * @brief
	 * the id of variable 
	 * */
	aclspv_id_t	m_id;
	aclspv_id_t	m_type_id;
	aclspv_id_t	m_etype_id;	/** element type */
	aclspv_id_t	m_ptr_type_id;

	/**
	 * @var	m_init_val
	 *
	 * @brief
	 * the initial value id 
	 * */
	aclspv_id_t	m_init_val;

	/**
	 * @brief
	 * [0,1]	mask_literal
	 * [2]		is predictable
	 * [3]		is allocated
	 * [4]		is m_is_occupied
	 * [5]		is owner
	 *
	 * @see ACLUTIL_CURSOR_VAR_
	 * */
	aclspv_wrd_t	m_mask;
	aclspv_wrd_t	m_num;
	aclspv_wrd_t	m_idx;
	aclspv_wrd_t	m_nxt;
} aclcursor_vardecl_t;

ae2f_extern  enum ACLSPV_COMPILE_ aclcursor_vardecl_evuclbl(
		const aclspv_wrdcount_t c_num_cursor, 
		const h_aclcursor_t	wr_cursor, 
		const h_aclspv_ctx_t	h_ctx,
		uintmax_t		c_stage_lvl
		);

aclspv_id_t aclcursor_vardecl_find_stagedval(
		const aclspv_wrdcount_t		c_num_cursor,
		const x_aclspv_vec* const ae2f_restrict h_cursors,
		const umax c_stage, 
		const aclspv_id_t c_ty
		);

#endif
