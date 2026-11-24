#ifndef	aclctx_err_h
#define aclctx_err_h

#include <aclspv.h>
#include <util/log.h>

#include <.aclspv/ctx/err.h>

/** error state check */
#define	aclctx_err_check(h)	_aclctx_err_log(h); ae2f_ifnezerr((h)->m_err)

#endif
