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

#ifndef	aclspv_usrcfg_h
#define aclspv_usrcfg_h

#include "./wrd.h"
#include "./ver.h"
#include "./enum/layout.h"

#include <ae2f/cc/qualifier.h>

#undef	_enum
#undef	_x

#define	_enum()			\
	_x(SYMBOL),			\
	_x(DUMP_DIAGNOSTICS),		\
	_x(KILL_ON_DIAGNOSTICS),	\
	_x(EXPLICIT_CAPABILITY),	\
	_x(EXPLICIT_EXTENSION),	\
	_x(EXPLICIT_EXTINST_IMPORT),	\
	_x(ERROR_UNASSIGNED_VAL_USED),	\
	_x(EXPORT_BY_DEFAULT),		\
	_x(INLINE_BY_DEFAULT),		\
	_x(ALLOW_RECURSION)

#define	_x(A)	ACLSPV_CTX_FLAGPAD0_##A

enum ACLSPV_CTX_FLAGPAD0_ {
	_enum()
};

#undef	_x
#define	_x(A)	ACLSPV_CTX_FLAG0_##A = 1u << ae2f_static_cast(aclspv_wrd_t, ACLSPV_CTX_FLAGPAD0_##A)

enum ACLSPV_CTX_FLAG0_ {
	_enum()
};

#undef	_x
#undef	_enum

#include <ae2f/cc/fnpretty.h>
#include <stddef.h>

/** user can define this */
typedef struct usrcfg_alloc_usr usrcfg_alloc_usr_t;

typedef struct {
	void*	m_usrdata;

	/** 
	 * r_mem could be used. 
	 * */
	void*	(*m_malloc)	(
			aclspv_wrd_t c_sz,
			aclspv_wrd_t* ae2f_restrict r_sz_real,
			void* ae2f_restrict *	r_mem,
			void*	h_usr
			);
	void	(*m_free)	(void* fini_mem, aclspv_wrd_t c_sz, void* h_usr);

	aclspv_wrd_t		m_max_push_size_limit;

	/** @brief see `ACLSPV_CTX_FLAG0_` */
	aclspv_wrd_t		m_flag0;

	/**
	 * @brief
	 * A output bitcode(spir-v)'s version.
	 * Use `aclspv_ver_spv` to specify it
	 * */
	aclspv_ver_spv_t	m_ver_spv;
	aclspv_ver_vk_t		m_ver_vk;

	/** default memory layout */
	aclspv_layout_t		m_layout_default;

	/** byte size of sizeof() operation */
	aclspv_wrd_t		m_byte_size_type;

	/** see SpvAddressingModel */
	aclspv_wrd_t		m_mdl_addr;

	/** see SpvMemoryModel */
	aclspv_wrd_t		m_mdl_mem;
} aclspv_usrcfg_t;	/** user can configurate this */

#endif
