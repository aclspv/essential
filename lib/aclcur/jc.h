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

#ifndef	aclcur_jc_h
#define aclcur_jc_h

#include <aclspv.h>

typedef struct aclcur_jc {
	aclspv_id_t	m_cond;
	aclspv_id_t	m_lbl_true;
	aclspv_id_t	m_lbl_false;
	aclspv_wrd_t	m_brw_true;
	aclspv_wrd_t	m_brw_false;

	/** 
	 * SpvOpSelectionMerge / SpvOpLoopMerge merge target.
	 * 0 = plain conditional branch, no merge instruction.
	 * */
	aclspv_id_t	m_merge;

	/** SpvOpLoopMerge continue target. 0 = selection merge. */
	aclspv_id_t	m_continue;

	/** selection / loop control mask. kept ignored (0). */
	aclspv_wrd_t	m_ctl;
} aclcur_jc_t;


aclspv_id_t	aclcur_cfg_jc(
		aclspv_id_t	c_cond,
		aclspv_id_t	c_lbl_true,
		aclspv_id_t	c_lbl_false,
		aclspv_wrd_t	c_brw_true,
		aclspv_wrd_t	c_brw_false,
		aclspv_id_t	c_merge_opt,
		aclspv_id_t	c_continue_opt,
		aclspv_wrd_t	c_ctl,
		h_aclspv_ctx_t	h_ctx
		);

#endif
