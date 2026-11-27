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

#ifndef	aclspv_ctx_sym_h
#define aclspv_ctx_sym_h

#include "./common.h"

#include <aclspv/id.h>
#include <aclspv/link.h>

#include <ae2f/Cxx/extern.h>

/** c should be len of `rd` EXCLUDING null terminator (\0) */
ae2f_extern ACLSPV_ABI_LINK aclspv_id_t aclspv_cfg_symbol(
		const char* ae2f_restrict rd,
		aclspv_wrd_t c,
		aclspv_id_t	c_new_opt,
		h_aclspv_ctx_t h
		);

#endif
