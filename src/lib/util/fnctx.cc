#ifndef	aclutil_fnctx_cc
#define	aclutil_fnctx_cc

#include <util/fnctx/defs.h>
#include <ae2f/pat/macrofunc.h>

ae2f_MAC() aclutil_fnctx_snap(aclutil_fnctx& r, const aclutil_fnctx c, const enum ACLUTIL_FNCTX_SNAPS_ c_snap) {
	(r).m_snaps[c_snap].m_idx_fnimpl	= (c).m_ctx->m_count.m_fnimpl;
	(r).m_snaps[c_snap].m_idx_fndef		= (c).m_ctx->m_count.m_fndef;
	(r).m_snaps[c_snap].m_idx_name		= (c).m_ctx->m_count.m_name;
}

ae2f_MAC() aclutil_fnctx_sync(aclutil_fnctx& r, const aclutil_fnctx c, const enum ACLUTIL_FNCTX_SNAPS_ c_snap) {
	(r).m_ctx->m_count.m_fnimpl		= (c).m_snaps[c_snap].m_idx_fnimpl;
	(r).m_ctx->m_count.m_fndef		= (c).m_snaps[c_snap].m_idx_fndef;
	(r).m_ctx->m_count.m_name		= (c).m_snaps[c_snap].m_idx_name;
}

#endif
