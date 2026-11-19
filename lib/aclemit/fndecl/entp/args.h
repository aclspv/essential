#ifndef	aclemit_fndecl_entp_args_h
#define aclemit_fndecl_entp_args_h

#include <aclspv.h>
#include <aclsym/entp.h>

struct	aclemit_fndecl_entp_arg_data {

	aclspv_wrd_t		m_idx_entp_buf_count;

	/** @brief context */
	h_aclspv_ctx_t		m_ctx;

	aclsym_entp_t		m_entp;
};


enum CXChildVisitResult aclemit_fndecl_entp_arg(CXCursor h_cur, CXCursor h_parent, CXClientData ae2f_restrict h_data);

#endif
