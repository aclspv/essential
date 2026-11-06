#ifndef	src_aclctx_newid_cc
#define src_aclctx_newid_cc

#include <aclctx/id.h>
#include <ae2f/pat/macrofunc.h>
#include <util/log.h>

ae2f_MAC() aclctx_newid_msg(aclspv_id_reserved_t& r_resid, h_aclspv_ctx_t h) {
	(r_resid) = aclctx_newid((h));
	dbg_prefix(); dbg_call(fprintf, (stderr, "new id: [%u]\n", (unsigned)(r_resid)));
}

#endif
