/** 
 * @file	getchild.h 
 * */

#ifndef	attr_getchild_h
#define attr_getchild_h

#include <clang-c/Index.h>
#include <ae2f/Keys.h>
#include <ae2f/c90/StdInt.h>
#include <ae2f/pat/dbg.h>
#include <aclspv/spvty.h>

typedef struct {
	CXCursor	m_ret;
	u32_least	m_num_ignore;
} s_attr_getchild_visit_t;

/** @param rdwr_cursor [s_attr_getchild_t] */
static enum CXChildVisitResult s_attr_getchild_visit(
		CXCursor c_cur, 
		ae2f_unused CXCursor c_parent, 
		CXClientData ae2f_restrict rdwr_cursor
		) {
	ae2fpat_assertassume_strict(rdwr_cursor);

	((s_attr_getchild_visit_t* ae2f_restrict)rdwr_cursor)->m_ret
		= c_cur;

	unless(((s_attr_getchild_visit_t* ae2f_restrict)rdwr_cursor)->m_num_ignore--) 
		return CXChildVisit_Break;

	return CXChildVisit_Recurse;
}

/**
 * @returns	when not 0, it is failed.
 * */
static CXCursor	s_attr_getchild(const CXCursor c_cur, const u32_least c_num_ignore) {
	s_attr_getchild_visit_t	GETCHILD;
	GETCHILD.m_num_ignore	= c_num_ignore;

	clang_visitChildren(
			c_cur
			, s_attr_getchild_visit
			, &GETCHILD);

	return GETCHILD.m_ret;
}

#endif
