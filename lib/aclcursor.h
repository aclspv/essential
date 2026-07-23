/** 
 * @file cursor.h 
 * @brief cursor master list 
 * */

#ifndef aclcursor_h
#define aclcursor_h

#include "./aclcursor/vardecl.h"
#include "./aclcursor/phi.h"

#include <ae2f/Keys.h>

#include <util/wrd.h>
#include <util/bind.h>
#include <util/literal.h>

enum ACLCURSOR_ {
	ACLCURSOR_VARDECL,
	ACLCURSOR_PHI,
	ACLCURSOR_PARENTP
};

typedef struct a_aclcursor {
	enum ACLCURSOR_	m_kind;

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
		aclcursor_vardecl_t	m_vardecl;
		aclcursor_phi_t		m_phi;

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

#endif
