/** @file cursor.c */

#include "./aclcursor.h"
#include "./aclmem.h"

#include <ae2f/bll.h>
#include <ae2f/pat/dbg.h>
#include <util/emitx.h>

aclspv_wrdcount_t aclutil_find_cursor(
		const aclspv_wrdcount_t			c_num_cursor, 
		const x_aclcursor* ae2f_restrict const	rd_cursors,
		const CXCursor				c_cursor_to_find
		)
{
	aclspv_wrdcount_t	IDX = c_num_cursor;
	ae2f_ifezerr(rd_cursors)	return c_num_cursor;

	while(IDX-- && !clang_equalCursors(rd_cursors[IDX].m_cursor, c_cursor_to_find)) {}
	return ae2f_bll_sel_bool1(aclspv_wrdcount_t, IDX < c_num_cursor, IDX, c_num_cursor);
}

aclspv_wrdcount_t aclutil_mk_cursor_base(
		const aclspv_wrdcount_t			_c_num_cursor,
		x_aclspv_vec* const ae2f_restrict	h_cursors,
		const CXCursor				c_cursor_to_add
		)
{
	const aclspv_wrd_t	c_num_cursor
		= ae2f_bll_sel_bool1(
				aclspv_wrd_t
				, (aclspv_wrd_t)((size_t)_c_num_cursor > h_cursors->m_sz / sizeof(x_aclcursor))
				, (aclspv_wrd_t)(h_cursors->m_sz / sizeof(x_aclcursor))
				, _c_num_cursor
				);

	const aclspv_wrdcount_t IDX 
		= aclutil_find_cursor(
				c_num_cursor
				, h_cursors->m_p
				, c_cursor_to_add
				);

	unless(IDX == c_num_cursor) return IDX;

	ae2f_ifnezerr_strict(s_aclmem_cgrow2(
				h_cursors
				, c_num_cursor + 1
				, sizeof(x_aclcursor)
				, ACLSPV_COMPILE_ALLOC_FAILED_CURSOR
				)) return c_num_cursor + 1;

	aclmemzero(&((x_aclcursor* ae2f_restrict)h_cursors->m_p)[c_num_cursor], sizeof(x_aclcursor));

	((x_aclcursor* ae2f_restrict)h_cursors->m_p)[c_num_cursor].m_cursor
		= c_cursor_to_add;

	return c_num_cursor ;
}
