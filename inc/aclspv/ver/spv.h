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

#ifndef	aclspv_ver_spv_h
#define aclspv_ver_spv_h

#include "./common.h"
#include <aclspv/wrd.h>

/** 
 * represents version c_mjr.c_mnr in 32-bits (aclspv_wrd_t)
 *
 * c_mjr: u8	; majour version
 * c_mnr: u8	; minour version
 * */
#define	aclspv_ver_spv(c_mjr, c_mnr)	\
	(aclspv_and_pad(c_mjr, 0xFF, 16) | aclspv_and_pad(c_mnr, 0xFF, 8))

typedef aclspv_wrd_t	aclspv_ver_spv_t;

#endif
