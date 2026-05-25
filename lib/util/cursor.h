/** 
 * @file cursor.h 
 * @brief cursor master list 
 * */

#ifndef util_cursor_h
#define util_cursor_h

#include <clang-c/Index.h>
#include <ae2f/Keys.h>

#include "./wrd.h"
#include "./bind.h"
#include "./literal.h"
#include <.aclspv/vec.h>

enum	ACLUTIL_CURSOR_VAR_ {
	ACLUTIL_CURSOR_VAR_LITERAL	= 0x3,
	ACLUTIL_CURSOR_VAR_PREDICTABLE	= 0x4,
	ACLUTIL_CURSOR_VAR_ALLOCATED	= 0x8,
	ACLUTIL_CURSOR_VAR_OCCUPIED	= 0x10,
	ACLUTIL_CURSOR_VAR_OWNER	= 0x20
};

typedef struct {
	/**
	 * @var		m_cursor
	 * @brief	`CXCursorKind` could be retrieved here
	 * */
	CXCursor	m_cursor;

	/**
	 * @var		m_data
	 * @brief	additional data
	 * */
	union {
		struct {
			util_literal	m_literal;
			umax		m_stage_lvl;

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
		}
		/** when cursor kind is arithmetic variable declaration */
		m_vardecl;

		struct {
			/** @brief label id for goto */
			aclspv_id_t	m_id;
		} 
		/** when cursor kind is goto label */
		m_goto_lbl;

		struct {
			util_bind	m_info;
		}
		/** when cursor kind is param decl */
		m_prm_decl;
	} m_data;
} util_cursor;

/**
 * find cursor
 * */
ae2f_extern  aclspv_wrdcount_t aclutil_find_cursor(
		const aclspv_wrdcount_t			c_num_cursor, 
		const util_cursor* ae2f_restrict const	rd_cursors,
		const CXCursor				c_cursor_to_find
		);


/** after allocation, the allocated node will be empty except for `c_cursor_to_add` since that's the key. */
ae2f_extern  aclspv_wrdcount_t aclutil_mk_cursor_base(
		const aclspv_wrdcount_t			c_num_cursor,
		x_aclspv_vec* const ae2f_restrict	h_cursors,
		const CXCursor				c_cursor_to_add
		);

ae2f_extern  enum ACLSPV_COMPILE_ aclutil_tell_cursor_lbl(
		const aclspv_wrdcount_t c_num_cursor, 
		util_cursor* ae2f_restrict const wr_cursor, 
		const h_util_ctx_t h_ctx,
		uintmax_t				c_stage_lvl
		);

aclspv_id_t aclutil_from_cursor_find_stagedval(
		const aclspv_wrdcount_t		c_num_cursor,
		const x_aclspv_vec* const ae2f_restrict h_cursors,
		const umax c_stage, 
		const aclspv_id_t c_ty
		);

ae2f_extern aclspv_wrdcount_t aclutil_assign_cursor_lval(
		util_cursor* ae2f_restrict const wr_cursor,
		const h_util_ctx_t			h_ctx,
		const aclspv_id_t			c_val,
		const aclspv_id_t			c_val_ty,
		const B_util_literal_t			c_masklit,
		const util_literal* ae2f_restrict	rd_literal_to_assign,
		util_literal*	ae2f_restrict		r_literal_opt
		);

#endif
