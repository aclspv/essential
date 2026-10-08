#ifndef	aclexpr_curreg_h
#define aclexpr_curreg_h

#include <aclcur/common.h>

/**
 * find the register node bound to a cursor.
 *
 * scans the cursor buffer for an ACLCUR_REG whose m_cur equals c_cur.
 * returns its id, or 0 when no such register exists yet.
 * */
static ae2f_inline_f aclspv_id_t aclexpr_curreg_find(CXCursor c_cur, h_aclspv_ctx_t h_ctx) {
	aclspv_wrd_t i;
	aclcur_t* ae2f_restrict p = (aclcur_t* ae2f_restrict)h_ctx->m_cursors.m_p;

	/** a null cursor must never match.
	 *
	 * clang_equalCursors reports two null cursors as equal, and registers
	 * are created with clang_getNullCursor() where no cursor applies (see
	 * the value-phi temporary in aclexpr/beforevisit/bool1.c). without this
	 * guard a lookup on a cursor that references nothing — an integer
	 * literal index, say — returned whichever such register came first, and
	 * the caller then loaded a value unrelated to the expression. */
	if(clang_Cursor_isNull(c_cur))
		return 0;

	for(i = 0; i < h_ctx->m_num_cursor; ++i) {
		if(
				p[i].m_class == ACLCUR_REG
				&& clang_equalCursors(p[i].m_cur, c_cur)
		  ) return p[i].m_id;
	}

	return 0;
}

#endif

