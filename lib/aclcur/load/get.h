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

#ifndef	aclcur_load_get_h
#define aclcur_load_get_h

#include "../common.h"

/** 
 * find the load id from function implementation and constant area.
 *
 * returns.m_id		would be non-zero only if it is found from runtime.
 * returns.m_val 	would be non-zero if c_load has been found somewhere.
 * found none, you've got nothing.
 * */
ae2f_ccpure aclcur_t aclcur_get_load(aclspv_vid_t c_load, h_aclspv_ctx_t h_ctx);

#endif
