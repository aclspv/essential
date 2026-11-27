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

#ifndef	aclcfg_ptr_h
#define aclcfg_ptr_h

#include <aclspv.h>

ae2f_ccpure static aclspv_wrd_t s_aclcfg_get_ptrsz(h_aclspv_ctx_t h) {
	switch(h->m_usrcfg.m_mdl_addr) {
		default:
			return 0;
		case SpvAddressingModelPhysical32:
			return 4;
		case SpvAddressingModelPhysical64:	
		case SpvAddressingModelPhysicalStorageBuffer64:
			return 8;
	}

	ae2f_unreachable();
}

#endif
