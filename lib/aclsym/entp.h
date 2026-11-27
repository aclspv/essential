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

/** @file aclsym/entp.h
 *
 * pre-emit entry-point symbols: what the entry-point emitter needs to know
 * about a function before its body is carved.
 * */

#ifndef aclsym_entp_h
#define aclsym_entp_h

#include <aclspv.h>
#include <aclsym/fn.h>

typedef struct {
	aclspv_id_t	m_id;
	aclspv_wrd_t	m_idx_push_constant;
	aclspv_wrd_t	m_descriptor_set;

	/** short handle for the declaration: clang_hashCursor.
	 *
	 * no CXCursor is stored, for the same reason aclsym_fn_t stores a mark: the
	 * tree belongs to the caller. this lets a later walk of the same translation
	 * unit recognise an entry point that is already in the module, instead of
	 * emitting a second OpEntryPoint for it. */
	aclspv_wrd_t	m_mark;

	/** enum ACLSYM_FN_FLAG0_: CONST and PURE only. an entry point is not
	 * a call target, so it has no function symbol and its purity snapshot
	 * lives here instead. */
	aclspv_wrd_t	m_flag0;
} aclsym_entp_t;

#endif
