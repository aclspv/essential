#ifndef	aclemit_fndecl_entp_args_h
#define aclemit_fndecl_entp_args_h

#include <aclspv.h>
#include <util/entp.h>

struct	aclemit_fndecl_entp_arg_data {

	aclspv_wrd_t		m_idx_entp_buf_count;

	/** @brief push constant id when push constant size is no 0 */
	aclspv_id_t		m_id_push_constant;
	aclspv_id_t		m_id_push_constant_struct_ty;

	/** @brief per-function push-constant packing-level register view (R1), m_ty patched at finalization */
	aclspv_id_t		m_id_push_constant_reg_view;

	aclspv_wrd_t		m_idx_push_constant_prm;

	/** @brief context */
	h_aclspv_ctx_t		m_ctx;

	aclutil_entp_t		m_entp;
};


enum CXChildVisitResult aclemit_fndecl_entp_arg(CXCursor h_cur, CXCursor h_parent, CXClientData ae2f_restrict h_data);

#endif
