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

#ifndef	aclmem_zero_cc
#define aclmem_zero_cc

#include <ae2f/cc/switchctl.h>
#include <ae2f/pat/macrofunc.h>

#if	!_ae2f_MACRO_GENERATED
typedef unsigned	idx_t;
typedef char		el_t;
#endif

ae2f_MAC((L, idx_t, )) aclmemzero_duff8(idx_t& cr_num, el_t*& wr_el, const el_t c) {
	idx_t L$$IDX = ((cr_num) + 0x7) >> 3;

	if((cr_num)) switch((cr_num) & 0x7) {
		case 0x0:
			do {
				*(wr_el)++ = (c); ae2f_fallthrough;
				case 0x7: *(wr_el)++ = (c); ae2f_fallthrough;
				case 0x6: *(wr_el)++ = (c); ae2f_fallthrough;
				case 0x5: *(wr_el)++ = (c); ae2f_fallthrough;
				case 0x4: *(wr_el)++ = (c); ae2f_fallthrough;
				case 0x3: *(wr_el)++ = (c); ae2f_fallthrough;
				case 0x2: *(wr_el)++ = (c); ae2f_fallthrough;
				case 0x1: *(wr_el)++ = (c);
			} while(--L$$IDX);
			break;

		default: ae2f_unreachable();
	}
}

#endif
