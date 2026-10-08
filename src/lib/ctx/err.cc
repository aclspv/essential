#ifndef _aclspv_src_lib_ctx_err_cc
#define _aclspv_src_lib_ctx_err_cc

#include <aclspv.h>
#include <ae2f/pat/macrofunc.h>

#include <util/log.h>

ae2f_MAC() aclctx_err(
		const h_aclspv_ctx_t h
		, const aclspv_compile_t c
		) {
	unless((h)->m_err) {
		(h)->m_err = c;
	}

	err_prefix(());
	err_call(fprintf, (stderr, "err %u on line %d\n"
				, ae2f_static_cast(unsigned, ((h)->m_err))
				, ae2f_static_cast(int, __LINE__))
			);
}

#endif
