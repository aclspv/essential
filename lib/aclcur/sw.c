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

#include "./sw.h"
#include "./common.h"
#include "./util.h"
#include "./load/get.h"

#include <aclctx/id.h>

/** is c the id of a value the module can name? constants live only in the
 * type section, so they have no node; their id is legal as an operand. */
static aclspv_bool_t s_sw_sel_known(aclspv_id_t c_sel, h_aclspv_ctx_t h_ctx) {
	aclcur_t b_sel = aclcur_get_load(c_sel, h_ctx);
	return (aclspv_bool_t)!!b_sel.m_val;
}

aclspv_id_t aclcur_cfg_sw(
		aclspv_id_t		c_sel,
		aclspv_id_t		c_default,
		aclspv_id_t		c_merge,
		aclspv_wrd_t		c_ctl,
		aclspv_wrd_t		c_num_words,
		const aclcur_sw_entry_t* c_entries,
		aclspv_wrd_t		c_num,
		h_aclspv_ctx_t		h_ctx
		) {
	aclspv_id_t	id_sw;
	aclspv_wrd_t	idx_head = 0;
	aclspv_wrd_t	idx_prv = 0;
	aclspv_wrd_t	i;
	h_aclcur_t	p_sw;
	h_aclcur_t	p_sel;

	ae2f_ifezerr(h_ctx)			return 0;
	ae2f_ifezerr(c_sel)			return 0;
	ae2f_ifezerr(c_default)			return 0;
	ae2f_ifezerr(c_merge)			return 0;
	if(c_num)
		ae2f_ifezerr(c_entries)		return 0;
	ae2f_ifnezerr_strict(c_num_words != 1 && c_num_words != 2)
		return 0;

	unless(s_sw_sel_known(c_sel, h_ctx))
		return 0;

	/** case entries first: the SW node must remain the tail of its block. */
	for(i = 0; i < c_num; ++i) {
		aclspv_id_t	id_case = aclcur_cfg_node(0, h_ctx);
		aclcur_idx_t	idx_case;
		h_aclcur_t	p_case;
		const aclspv_wrd2_t VAL = c_entries[i].m_val;

		ae2f_ifezerr(id_case)	return 0;
		ae2f_ifezerr(c_entries[i].m_lbl)	return 0;

		idx_case = aclcur_idx_of(id_case, h_ctx);
		ae2f_ifnezerr_strict(idx_case == ACLCUR_IDX_INVAL)
			return 0;

		p_case = aclcur_get_node(id_case, h_ctx);
		ae2f_ifezerr(p_case)	return 0;

		p_case->m_class			= ACLCUR_SW_CASE;
		p_case->m_d.m_sw_case.m_val_lo	= (aclspv_wrd_t)(VAL & 0xFFFFFFFFu);
		p_case->m_d.m_sw_case.m_val_hi	= ae2f_static_cast(aclspv_wrd_t, VAL >> 32u);
		p_case->m_d.m_sw_case.m_lbl	= c_entries[i].m_lbl;
		p_case->m_d.m_sw_case.m_nxt	= 0;

		if(c_num_words == 1)
			p_case->m_d.m_sw_case.m_val_hi = 0;

		if(idx_prv) {
			h_aclcur_t p_prv = aclcur_get_node_idx(idx_prv, h_ctx);

			ae2f_ifezerr(p_prv)
				return 0;
			p_prv->m_d.m_sw_case.m_nxt = idx_case;
		} else {
			idx_head = idx_case;
		}

		idx_prv = idx_case;
	}

	id_sw = aclcur_cfg_node(0, h_ctx);
	ae2f_ifezerr(id_sw)
		return 0;

	p_sw = aclcur_get_node(id_sw, h_ctx);
	ae2f_ifezerr(p_sw)
		return 0;

	p_sw->m_class			= ACLCUR_SW;
	p_sw->m_d.m_sw.m_sel		= c_sel;
	p_sw->m_d.m_sw.m_default	= c_default;
	p_sw->m_d.m_sw.m_merge		= c_merge;
	p_sw->m_d.m_sw.m_ctl		= c_ctl;
	p_sw->m_d.m_sw.m_num_words	= c_num_words;
	p_sw->m_d.m_sw.m_cases_head	= idx_head;
	p_sw->m_d.m_sw.m_num_case	= c_num;

	p_sel = aclcur_get_node(c_sel, h_ctx);
	if(p_sel)
		(void)++p_sel->m_num_ref;

	return id_sw;
}
