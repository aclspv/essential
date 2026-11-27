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

#ifndef	aclcur_util_h
#define aclcur_util_h

#include "./common.h"

ae2f_ccpure static ae2f_inline_f aclcur_t* s_get_buf(const x_aclspv_ctx* ae2f_restrict h_ctx) {
	return h_ctx->m_cursors.m_p;
}

ae2f_ccpure static ae2f_inline_f const aclcur_t* s_get_buf_c(const x_aclspv_ctx* ae2f_restrict h_ctx) {
	return h_ctx->m_cursors.m_p;
}

#endif
