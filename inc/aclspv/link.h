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

#ifndef	aclspv_link_h
#define aclspv_link_h

#include <ae2f/cc/dylink.h>

/**
 * @def ACLSPV_ABI_IMPL	
 * @brief Implementation as ABI
 *
 * @def ACLSPV_ABI_DECL	
 * @brief Declaration as ABI
 * */

#ifndef	ACLSPVLIB
#	define	ACLSPVLIB	0
#endif

#if	__ACLSPV_SHARED
#	if ACLSPVLIB
#		define ACLSPV_ABI_LINK	ae2f_impl
#	else
#		define ACLSPV_ABI_LINK	ae2f_decl
#	endif
#else
#	define	ACLSPV_ABI_LINK
#endif

#endif
