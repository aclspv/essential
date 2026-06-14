#ifndef	aclexpr_aftervisit_util_h
#define	aclexpr_aftervisit_util_h

#define CTX(rdwr_data)			(rdwr_data)->m_ctx
#define EXPRSTCK_COUNT(rdwr_data)	(rdwr_data)->m_exprcount
#define	RET_LITERAL_REF(rdwr_data)	(rdwr_data)->m_ret_literal

#define	CMDSTCK_SCALE(CTX)	(CTX)->m_tmp.m_v1
#define	CMDCOUNTERSTCK(CTX)	(CTX)->m_tmp.m_v2

#endif
