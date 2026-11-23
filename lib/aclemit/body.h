#ifndef	aclemit_body_h
#define	aclemit_body_h

#include <acldef.h>

typedef struct { h_aclspv_ctx_t m_ctx; aclspv_vid_t m_vid; } aclemit_body_t;
enum CXChildVisitResult aclemit_body(CXCursor h_cur, CXCursor h_parent, CXClientData h_ctx);

#endif
