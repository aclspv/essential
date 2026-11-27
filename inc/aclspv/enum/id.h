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

#ifndef	aclspv_enum_id_h
#define aclspv_enum_id_h

/** 
 * type id. 
 * enum is ordered by arithmetic cast priority.
 * */
enum ACLSPV_ID_ {
	ACLSPV_ID_VOID = 1, 
	ACLSPV_ID_BOOL,

	ACLSPV_ID_U8,
	ACLSPV_ID_I8,

	ACLSPV_ID_U16,
	ACLSPV_ID_I16,

	ACLSPV_ID_U32,
	ACLSPV_ID_I32,

	ACLSPV_ID_U64,
	ACLSPV_ID_I64,

#if	__ACLSPV_SCLR_F8_E4M3
	ACLSPV_ID_F8_E4M3,
#endif

#if	__ACLSPV_SCLR_F8_E5M2
	ACLSPV_ID_F8_E5M2,
#endif

#if	__ACLSPV_SCLR_F16
	ACLSPV_ID_F16,
#endif
#if	__ACLSPV_SCLR_BF16
	ACLSPV_ID_BF16,
#endif
	ACLSPV_ID_F32,
	ACLSPV_ID_F64,

	ACLSPV_ID_END
};

#endif
