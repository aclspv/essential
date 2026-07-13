#ifndef	aclemit_fndecl_entp_args_h
#define aclemit_fndecl_entp_args_h

#include <aclspv.h>
#include <util/entp.h>

struct	aclemit_fndecl_entp_arg_data {
	/** @brief push constant size */
	aclspv_wrdcount_t	m_push_size;
	aclspv_wrdcount_t	m_push_size_max;

	/** @brief bind index */ 
	aclspv_wrdcount_t	m_idx_bind;

	/** @brief argument index */
	aclspv_wrdcount_t	m_idx_arg;

	/** @brief io argument index */
	aclspv_wrdcount_t	m_idx_ioarg;

	/** @brief interface count */
	aclspv_wrdcount_t	m_num_interface;

	aclspv_wrd_t		m_idx_entp_buf_count;

	/** @brief push constant id when push constant size is no 0 */
	aclspv_id_t		m_id_push_constant;

	/** @brief context */
	h_aclspv_ctx_t		m_ctx;

	aclutil_entp_t		m_entp;
};


enum CXChildVisitResult aclemit_fndecl_entp_arg(CXCursor h_cur, CXCursor h_parent, CXClientData ae2f_restrict h_data);

#endif
