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

/** @file wrdemit.h @brief word emission utilities */

#ifndef aclutil_wrdemit_h
#define aclutil_wrdemit_h


#include <ae2f/c90/lmts.h>
#include <aclspv.h>
#include <aclmem.h>

#include <.aclspv/util/vcopr.h>
#include <acldef.h>
#include <util/log.h>

#include <assert.h>


/** size_t for spir-v format */
typedef	aclspv_wrdcount_t	aclspv_sz_t;
#define	aclspv_sizeof(a)	((aclspv_sz_t)(sizeof(a)))

/** 
 * @def 	aclutil_sz_to_count 
 * @brief	byte size to word count
 * */
#define aclutil_sz_to_count(c_sz)	\
	(((aclspv_sz_t)(c_sz) + (aclspv_sizeof(aclspv_wrd_t)) - 1u) / aclspv_sizeof(aclspv_wrd_t))


/**
 * @def		aclutil_count_to_sz
 * @brief	word count to byte size
 * */
#define aclutil_count_to_sz(c_count)	(((aclspv_sz_t)(c_count)) * (aclspv_sizeof(aclspv_wrd_t)))

/**
 * @def		aclutil_get_wrd_of_vec
 * @brief	get word buffer from vector
 * */
#define aclutil_get_wrd_of_vec(vec)	((aclspv_wrd_t* ae2f_restrict)((vec)->m_p))

/**
 *
 * @fn		emit_wrd
 * @brief	try emitting `c_wrd` on `h_vec`
 * @return	new c_wrdcount available. 0 when failed allocating.
 * */
static ae2f_inline_f aclspv_sz_t aclutil_emit_wrd(
		x_aclspv_vec* ae2f_restrict const h_wrds, 
		const aclspv_sz_t c_wrdcount, 
		const aclspv_wrd_t c_wrd
		)
{
	ae2f_ifnezerr_strict(0u != s_aclmem_cgrow2(
			h_wrds
			, c_wrdcount + 1u
			, sizeof(aclspv_wrd_t)
			, ACLSPV_COMPILE_ALLOC_FAILED_EMIT_WRD
			)) {
		err_puts("failed");
		return 0;
	}

	ae2f_ifneznrm_strict(h_wrds->m_p && (aclutil_sz_to_count(h_wrds->m_sz) <= ACLSPV_MASK_WRD)) {
		aclutil_get_wrd_of_vec(h_wrds)[c_wrdcount] = c_wrd;
		return c_wrdcount + 1u;
	}

	return 0u;
}

#define	aclspv_mk_noprnds(c_num_opprm)	\
	((((aclspv_wrd_t)((c_num_opprm) + 1u)) << 16u) & (aclspv_wrd_t)ACLSPV_MASK_NOPRNDS)

#define	aclspv_opcode_to_wrd(c_opcode, c_num_opprm)	\
	((aclspv_wrd_t)(((aclspv_wrd_t)(c_opcode) & ACLSPV_MASK_OPCODE) | aclspv_mk_noprnds(c_num_opprm)))
/**
 * @def		emit_opcode
 * @brief	try emit opcode with num_opprm
 * */
#define	emit_opcode(h_wrds, c_wrdcount, c_opcode, c_num_opprm_opt)	\
	(aclutil_emit_wrd(h_wrds, c_wrdcount, aclspv_opcode_to_wrd(c_opcode, c_num_opprm_opt)))


#define aclspv_get_oprnd_count_for_opcode(c_wrd)	\
	(((c_wrd) >> 16u) & 0xFFFFu)

#define set_oprnd_count_for_opcode(cr_wrd, c_num_opprm)	\
	((cr_wrd) = aclspv_opcode_to_wrd(((cr_wrd) & ACLSPV_MASK_OPCODE), ((aclspv_num_opprm_t)(c_num_opprm))))


#include <string.h>

/**
 * @fn		emit_str
 * @brief	emit string with word size padded.
 * @returns	0 when failed.
 * */
ae2f_inline static aclspv_sz_t aclutil_emit_str(
		x_aclspv_vec* ae2f_restrict const h_wrds, 
		const aclspv_sz_t c_wrdcount, 
		const char* ae2f_restrict const rd_str
		)
{
	/** string len: null included. */
	const size_t	len_		= strlen(rd_str) + 1u;
	const aclspv_sz_t	len		= ae2f_expected_strict(len_ > (size_t)0xFFFFFFFFu) ? (aclspv_sz_t)0u : (aclspv_sz_t)len_;
	const aclspv_sz_t	pad_wrds	= (len + 3u) >> 2u;

	ae2f_expected_but_else(len)		return 0u;

	ae2f_ifnezerr_strict(0u != s_aclmem_cgrow2(
				h_wrds
				, pad_wrds + c_wrdcount
				, sizeof(aclspv_wrd_t)
				, ACLSPV_COMPILE_ALLOC_FAILED_EMIT_WRD
				)) return 0u;

	aclmemzero(&aclutil_get_wrd_of_vec(h_wrds)[c_wrdcount], aclutil_count_to_sz(pad_wrds));
	aclmemcpy(&aclutil_get_wrd_of_vec(h_wrds)[c_wrdcount], rd_str, len);

	return c_wrdcount + pad_wrds;
}

#endif
