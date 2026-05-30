#ifndef aclattr_getkinder_h
#define aclattr_getkinder_h

#include <clang-c/Index.h>
#include <util/wrd.h>
#include <ae2f/pat/dbg.h>

struct aclattr_getkinder {
	CXCursor* ae2f_restrict	m_cursor;
	enum CXChildVisitResult	m_res;

	aclspv_wrd_t		m_num_wanted;
	aclspv_wrd_t		m_num_grepped;
};

/**
 * @param	h_getkinder : aclattr_getkinder*
 * */
static enum CXChildVisitResult s_aclattr_getkinder_visit(
		CXCursor c_cur, 
		ae2f_unused CXCursor unused,
		CXClientData ae2f_restrict h_getkinder
		) {
	struct aclattr_getkinder* ae2f_restrict P_GETKINDER 
		= (struct aclattr_getkinder* ae2f_restrict)(h_getkinder);

	ae2fpat_assertassume(P_GETKINDER);
	ae2fpat_assertassume(P_GETKINDER->m_cursor);

	if(P_GETKINDER->m_num_wanted <= P_GETKINDER->m_num_grepped)
		return CXChildVisit_Break;


	P_GETKINDER->m_cursor[P_GETKINDER->m_num_grepped++]
		= c_cur;

	return P_GETKINDER->m_res;
}

#endif
