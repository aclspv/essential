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

#ifndef aclutil_cxtp_h
#define	aclutil_cxtp_h

#include <ae2f/Keys.h>

#include "./aclty/arr.h"
#include "./aclty/vec.h"
#include "./aclty/ptr.h"
#include "./aclty/struct.h"
#include "./aclty/fn.h"
#include "./aclty/by_cursor.h"
#include "./aclty/simd.h"

ae2f_ccpure	aclty_t*	aclty_find(const aclspv_id_t c, const h_aclspv_ctx_t h_ctx);
aclconst_specsz			aclty_getsize(const aclspv_id_t c, const h_aclspv_ctx_t h_ctx);
ae2f_ccpure	aclspv_wrd_t	aclty_getalign(const aclspv_id_t c, const h_aclspv_ctx_t h_ctx);


#endif
