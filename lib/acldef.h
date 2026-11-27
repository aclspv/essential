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

#ifndef	acldef_h
#define acldef_h

/** iterating types */
typedef int		aclspv_iter_t;
typedef unsigned int	aclspv_uiter_t;

#include <aclspv.h>
#include <ae2f/c90/stdint/shorten.h>
#include <ae2f/cc/inline.h>
#include <ae2f/cc/expect/naive.h>
#include <ae2f/Keys.h>

#if	__ACLSPV_SCLR_F16
#define	ACLSPV_ID_F16_OR_Z	ACLSPV_ID_F16
#elif	__ACLSPV_SCLR_BF16
#define	ACLSPV_ID_F16_OR_Z	ACLSPV_ID_BF16
#else
#define	ACLSPV_ID_F16_OR_Z	0
#endif

#if	__ACLSPV_SCLR_F8_E4M3
#define	ACLSPV_ID_F8_OR_Z	ACLSPV_ID_F8_E4M3
#elif	__ACLSPV_SCLR_F8_E5M2
#define	ACLSPV_ID_F8_OR_Z	ACLSPV_ID_F8_E5M2
#else
#define	ACLSPV_ID_F8_OR_Z	0
#endif

/**
 * @typedef	aclspv_opcode_t
 * @brief	integer as operation code
 * */
#ifndef aclspv_opcode_t
#define aclspv_opcode_t	aclspv_opcode_t
typedef uint_least16_t	aclspv_opcode_t;
#endif

typedef char ACLSPV_CHECK_OPCODE[sizeof(aclspv_opcode_t) >= 2u ? 1 : -1];

/**
 * @def		ACLSPV_MASK_OPCODE
 * @brief	mask for opcode
 * */
#define	ACLSPV_MASK_OPCODE	((aclspv_wrd_t)0x0000ffffu)
/**
 * @def		ACLSPV_MASK_NOPRNDS
 * @brief	mask for number of operands
 * */
#define ACLSPV_MASK_NOPRNDS	((aclspv_wrd_t)0xffff0000u)

/**
 * @def		ACLSPV_MASK_WRD
 * @brief	mask for word
 * */
#define ACLSPV_MASK_WRD		((aclspv_wrd_t)0xffffffffu)

typedef char ACLSPV_CHECK_MASK_WRD[ACLSPV_MASK_WRD == (((aclspv_wrd_t)0u) - ((aclspv_wrd_t)1u)) ? 1 : -1];

/**
 * @typedef	aclspv_num_opprm
 * @brief	number of parameter of opcode (aka `aclspv_opcode_t`)
 * */
typedef aclspv_wrd_t	aclspv_num_opprm_t;

/** 
 * any value, which has data type.
 * it could be runtime & constant.
 *
 * see aclty_id_t
 * */
typedef aclspv_id_t	aclspv_vid_t;

/** 
 * this is an id but not by master but the slave one.
 * see x_aclspv_ctx::m_ctx_slave.
 *
 * NEVER assume this type stores a value is valid an to be emitted.
 * */
typedef aclspv_id_t	aclspv_subid_t;

/** 
 * reserved id 
 * does not have any role yet.
 * */
typedef aclspv_id_t	aclspv_id_reserved_t;

#define	_aclspv_enumwrapper(u, ty, test, en)	\
	typedef u ty; typedef u test[sizeof(en) <= sizeof(ty) ? 1 : -1]

_aclspv_enumwrapper(aclspv_wrd_t, aclspv_cxcurkind_t, aclspv_cxcurkind_test, enum CXCursorKind);
_aclspv_enumwrapper(aclspv_wrd_t, aclspv_cxunaryopkind_t, aclspv_cxunaryopkind_test, enum CXUnaryOperatorKind);
_aclspv_enumwrapper(aclspv_wrd_t, aclspv_cxbinopkind_t, aclspv_cxbinopkind_test, enum CXBinaryOperatorKind);
_aclspv_enumwrapper(aclspv_wrd_t, aclspv_cxevalreskind_t, aclspv_cxevalreskind_test, CXEvalResultKind);
_aclspv_enumwrapper(aclspv_wrd_t, aclspv_cxtykind_t, aclspv_cxtykind_test, enum CXTypeKind);

#endif
