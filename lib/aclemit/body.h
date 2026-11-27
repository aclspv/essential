/*
 * Copyright 2026 dalmurii
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * See the LICENSE file for the full text, including LLVM Exceptions.
 */

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
