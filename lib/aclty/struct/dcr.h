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

#ifndef aclutil_cxtp_struct_dcr_h
#define aclutil_cxtp_struct_dcr_h

#include "../common.h"

/** STRUCT DECORATION */
aclty_t* aclty_cfg_struct_dcr(aclty_t* ae2f_restrict rdwr_struct, SpvDecoration c_decor, h_aclspv_ctx_t h_ctx);
aclty_t* aclty_cfg_struct_dcr1(aclspv_id_t c_id, SpvDecoration c_decor, h_aclspv_ctx_t h_ctx);

#endif
