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

#ifndef	aclcur_j_h
#define aclcur_j_h

#include <aclcur.h>

#include "./j/emit.h"
#include "./lbl.h"

typedef struct {
	aclspv_wrd_t	m_lbl;
} aclcur_j_t;

typedef aclcur_id_t aclcur_jid_t;

aclcur_jid_t	aclcur_cfg_jmp(aclcur_lblid_t c_lbl, h_aclspv_ctx_t h_ctx);

#endif
