#ifndef	aclexpr_aftervisit_util_h
#define	aclexpr_aftervisit_util_h

#define CTX(rdwr_data)			(rdwr_data)->m_ctx
#define EXPRSTCK_COUNT(rdwr_data)	(rdwr_data)->m_exprcount
#define	RET_LITERAL_REF(rdwr_data)	(rdwr_data)->m_ret_literal

#define	CMDSTCK_SCALE(CTX)	(CTX)->m_body.m_bootscale
#define	CMDCOUNTERSTCK(CTX)	(CTX)->m_body.m_cmdcountstck

#endif
