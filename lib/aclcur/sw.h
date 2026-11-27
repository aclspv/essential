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

#ifndef	aclcur_sw_h
#define aclcur_sw_h

#include <aclcur.h>
#include "./class.h"

/** one `case <literal>: target` entry of an ACLCUR_SW node.
 *
 * the entry is linked from its predecessor by cursor-buffer index; index 0
 * terminates the chain (the root label can never be a case entry). */
typedef struct aclcur_sw_case {
	aclspv_wrd_t	m_val_lo;
	aclspv_wrd_t	m_val_hi;

	/** target label id. */
	aclspv_id_t	m_lbl;

	/** cursor-buffer index of the next entry, 0 = none. */
	aclspv_wrd_t	m_nxt;
} aclcur_sw_case_t;

/** SpvOpSwitch terminator.
 *
 * emission shape: [op][selector][default][literal, label]...
 * `m_num_words` is the literal width in words: 1 for a selector of up to
 * 32 bits, 2 for a 64-bit selector. */
typedef struct aclcur_sw {
	aclspv_id_t	m_sel;
	aclspv_id_t	m_default;

	/** SpvOpSelectionMerge merge target. */
	aclspv_id_t	m_merge;

	/** selection control mask. kept ignored (0). */
	aclspv_wrd_t	m_ctl;

	aclspv_wrd_t	m_num_words;

	/** cursor-buffer index of the first case entry, 0 = no case. */
	aclspv_wrd_t	m_cases_head;
	aclspv_wrd_t	m_num_case;
} aclcur_sw_t;

/** one case supplied to aclcur_cfg_sw: the evaluated literal and its target. */
typedef struct aclcur_sw_entry {
	aclspv_wrd2_t	m_val;
	aclspv_id_t	m_lbl;
} aclcur_sw_entry_t;

/** carve one switch terminator.
 *
 * the case entries are carved BEFORE the SW node, so the SW node stays the
 * tail of its label (the control-flow walks read the tail to find the
 * successors). target labels may still be forward references: their
 * references are discovered when aclcur_cfg_lbl() counts the edges.
 *
 * c_entries[i].m_val holds the zero-extended or two's-complement literal;
 * only the low `c_num_words` words are kept.
 *
 * returns the SW node id, or 0 on failure. */
aclspv_id_t aclcur_cfg_sw(
		aclspv_id_t		c_sel,
		aclspv_id_t		c_default,
		aclspv_id_t		c_merge,
		aclspv_wrd_t		c_ctl,
		aclspv_wrd_t		c_num_words,
		const aclcur_sw_entry_t* c_entries,
		aclspv_wrd_t		c_num,
		h_aclspv_ctx_t		h_ctx
		);

#include "./sw/emit.h"

#endif
