#ifndef	aclattr_ifpart_h
#define aclattr_ifpart_h

#include <aclspv.h>
#include <ae2f/Keys.h>
#include <ae2f/bll.h>

struct s_attr_ifpart_t {
	CXCursor		m_parts[3];
	h_aclspv_ctx_t		m_ctx;
	aclspv_wrdcount_t	m_n; /** 0, 1, 2, 3 */ 
	aclspv_wrdcount_t	m_n_dummy; /** 0, 1, 2, 3 */ 
	aclspv_id_t		m_b_true;
	aclspv_id_t		m_b_false;
	aclspv_id_t		m_b_default;
};

/** @param wr_data: s_attr_ifpart */
static enum CXChildVisitResult s_attr_ifpart(CXCursor h_cur, ae2f_unused CXCursor h_parent, CXClientData ae2f_restrict wr_data) {
	(((struct s_attr_ifpart_t* ae2f_restrict)(wr_data)))->m_n++
		[((struct s_attr_ifpart_t* ae2f_restrict)(wr_data))->m_parts]
		= h_cur;

	(((struct s_attr_ifpart_t* ae2f_restrict)(wr_data)))->m_n_dummy++;

	if(((struct s_attr_ifpart_t* ae2f_restrict)(wr_data))->m_n == 3) {
		((struct s_attr_ifpart_t* ae2f_restrict)(wr_data))->m_b_false
			= ((struct s_attr_ifpart_t* ae2f_restrict)(wr_data))->m_ctx->m_id++;
	}

	return CXChildVisit_Continue;
}

#endif 
