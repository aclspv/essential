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
