/** 
 * @file cursor.h 
 * @brief cursor master list 
 * */

#ifndef util_cursor_h
#define util_cursor_h

#include "./aclcursor/vardecl.h"

#include <ae2f/Keys.h>

#include <util/wrd.h>
#include <util/bind.h>
#include <util/literal.h>
#include <.aclspv/vec.h>

enum	ACLUTIL_CURSOR_VAR_ {
	ACLUTIL_CURSOR_VAR_LITERAL	= 0x3,
	ACLUTIL_CURSOR_VAR_PREDICTABLE	= 0x4,
	ACLUTIL_CURSOR_VAR_ALLOCATED	= 0x8,
	ACLUTIL_CURSOR_VAR_OCCUPIED	= 0x10,
	ACLUTIL_CURSOR_VAR_OWNER	= 0x20
};


typedef struct a_aclcursor {
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
		aclcursor_vardecl_t m_vardecl;

		struct {
			/** @brief label id for goto */
			aclspv_id_t	m_id;
		} 
		/** when cursor kind is goto label */
		m_goto_lbl;

		struct {
			aclutil_bind	m_info;
		}
		/** when cursor kind is param decl */
		m_prm_decl;
	} m_data;
} x_aclcursor;

/**
 * find cursor
 * */
ae2f_extern  aclspv_wrdcount_t aclutil_find_cursor(
		const aclspv_wrdcount_t			c_num_cursor, 
		const x_aclcursor* ae2f_restrict const	rd_cursors,
		const CXCursor				c_cursor_to_find
		);


/** after allocation, the allocated node will be empty except for `c_cursor_to_add` since that's the key. */
ae2f_extern  aclspv_wrdcount_t aclutil_mk_cursor_base(
		const aclspv_wrdcount_t			c_num_cursor,
		x_aclspv_vec* const ae2f_restrict	h_cursors,
		const CXCursor				c_cursor_to_add
		);

ae2f_extern aclspv_id_t aclutil_assign_cursor_lval(
		x_aclcursor* ae2f_restrict const wr_cursor,
		const h_aclspv_ctx_t			h_ctx,
		const aclspv_id_t			c_val,
		const aclspv_id_t			c_val_ty,
		const aclutil_ltrlmask_t			c_masklit,
		const aclutil_literal* ae2f_restrict	rd_literal_to_assign,
		aclutil_literal*	ae2f_restrict		r_literal_opt
		);

#endif
