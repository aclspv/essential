#ifndef	aclemit_body_h
#define	aclemit_body_h

#include <acldef.h>
#include <aclty/common.h>

typedef struct { h_aclspv_ctx_t m_ctx; aclspv_vid_t m_vid; aclty_id_t m_ty; } aclemit_body_t;
enum CXChildVisitResult aclemit_body(CXCursor h_cur, CXCursor h_parent, CXClientData h_ctx);

/**
 * @brief	carve one GNU statement expression.
 *
 * emits the statement list of c_stmtexpr into the current control-flow
 * context and returns the value id of its last expression; its type id is
 * stored through wr_ty. the outer expression evaluator owns the scratch
 * scale, so it is detached while the statements run and restored on return:
 * a statement expression nested in an expression may evaluate expressions of
 * its own.
 *
 * returns 0 on failure (h_ctx->m_err is set).
 * */
aclspv_id_t aclemit_body_stmtexpr(CXCursor c_stmtexpr, aclty_id_t* wr_ty, h_aclspv_ctx_t h_ctx);

#endif
