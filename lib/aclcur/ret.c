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

#include "./ret.h"
#include "./common.h"
#include "./util.h"

aclspv_id_t aclcur_cfg_ret(
		aclspv_id_t c_val_opt,
		h_aclspv_ctx_t h_ctx
		) {
	h_aclcur_t p_ret;
	aclspv_id_t	id_ret;

	ae2f_ifezerr(id_ret = aclcur_cfg_node(
				0, h_ctx
				))
		return 0;

	ae2f_ifezerr(p_ret = aclcur_get_node(id_ret, h_ctx))
		return 0;

	p_ret->m_class	= ACLCUR_RET;
	p_ret->m_val	= c_val_opt;

	/** the returned value keeps its producing instruction alive: an op with
	 * no other user (the last one in an expression) is dropped by its
	 * emitter's refcount gate otherwise. */
	if(c_val_opt) {
		h_aclcur_t p_val = aclcur_get_node(c_val_opt, h_ctx);

		if(p_val)
			(void)++p_val->m_num_ref;
	}

	return id_ret;
}

aclspv_id_t aclcur_cfg_unreachable(h_aclspv_ctx_t h_ctx) {
	h_aclcur_t p;
	aclspv_id_t id;

	ae2f_ifezerr(id = aclcur_cfg_node(0, h_ctx))
		return 0;

	ae2f_ifezerr(p = aclcur_get_node(id, h_ctx))
		return 0;

	p->m_class	= ACLCUR_UNREACHABLE;
	p->m_val	= 0;

	return id;
}
