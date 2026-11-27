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

#include "./emit.h"
#include <aclctx/err.h>
#include <aclutil/wrdemit.h>
#include "./util.h"
#include "./flag0.h"
#include "./phi.h"
#include "./j/emit.h"
#include "./jc/emit.h"
#include "./sw/emit.h"
#include "./ret/emit.h"
#include "./op/emit.h"
#include "./opX/emit.h"

#include <.aclspv/util/mkname.h>

aclspv_compile_t aclcur_emit(h_aclspv_ctx_t h_ctx) {
	aclspv_wrd_t i;

	/** assumable. it should be internal. */
	ae2f_assume(h_ctx);

	aclctx_err_check(h_ctx)
		return h_ctx->m_err;

	for(i = 0; i < h_ctx->m_num_cursor; ++i) {
		h_aclcur_t p = &s_get_buf(h_ctx)[i];

		/** if emitting none, continue. otherwise, break the switch. */
		switch(p->m_class) {
			case ACLCUR_RESERVED:
				/** phis are emitted through their owning LOAD node. */
			case ACLCUR_PHI:
			/** switch case entries are data of their SW node. */
			case ACLCUR_SW_CASE:
				continue;

			case ACLCUR_LBL:
				{
					const aclspv_wrd_t c_blk = p->m_d.m_lbl.m_nop;

					aclctx_err_check2(h_ctx, EMIT_aclcur_lbl(p, h_ctx))
						return h_ctx->m_err;

					/** the label was powered off for lack of incoming
					 * edges and its sweep has marked the region dead:
					 * elide the block's instructions, or they would be
					 * emitted outside any block. registers still emit —
					 * their storage (OpVariable) goes to the declaration
					 * stream and may be referenced by live code. */
					unless(p->m_num_ref
							|| (p->m_flag0 & aclcur_flag0_lbl(ACLCUR_FLAG0_LBL_ROOT))) {
						aclspv_wrd_t j;

						for(j = 1; j < c_blk; ++j) {
							h_aclcur_t p_mid = &s_get_buf(h_ctx)[i + j];

							if(p_mid->m_class == ACLCUR_REG
									&& p_mid->m_num_ref
									&& p_mid->m_d.m_reg.m_in.m_emit_reg) {

								aclctx_err_check2(h_ctx, p_mid->m_d.m_reg.m_in.m_emit_reg(p_mid->m_id, h_ctx))
									return h_ctx->m_err;
							}
						}

						i += c_blk - 1;
					}

					/** the block's value-flow phi chains (created by the
					 * load resolution at the cursor tail) head the block:
					 * OpPhi must precede any other function-body
					 * instruction of the block. a powered-off (swept
					 * dead) label must not hoist them: its instructions
					 * are elided, an OpPhi would land outside any live
					 * block. */
					unless(p->m_num_ref
							|| (p->m_flag0 & aclcur_flag0_lbl(ACLCUR_FLAG0_LBL_ROOT)))
						break;

					{
						aclspv_wrd_t j;
						aclspv_id_t emd[64];
						aclspv_wrd_t ne = 0;

						for(j = 0; j < h_ctx->m_num_cursor; ++j) {
							h_aclcur_t q = &s_get_buf(h_ctx)[j];
							aclspv_id_t head;
							aclspv_wrd_t k;

							if(q->m_class != ACLCUR_PHI
							|| !(q->m_flag0 & aclcur_flag0_phi(ACLCUR_FLAG0_PHI_SELF))
							|| q->m_lbl != p->m_id)
								continue;

							head = aclcur_phi_head(q->m_id, h_ctx);
							ae2f_ifezerr(head)
								continue;

							for(k = 0; k < ne; ++k)
								if(emd[k] == head)
									break;
							if(k != ne)
								continue;

							aclctx_err_check2(h_ctx, aclcur_phi_emit(head, h_ctx))
								return h_ctx->m_err;

							if(ne < 64)
								emd[ne++] = head;
						}
					}

					break;
				}
			case ACLCUR_J:   _aclctx_err(h_ctx, EMIT_aclcur_jmp(p, h_ctx)); break;
			case ACLCUR_JC:  _aclctx_err(h_ctx, EMIT_aclcur_jc(p, h_ctx)); break;
			case ACLCUR_SW:  _aclctx_err(h_ctx, EMIT_aclcur_sw(p, h_ctx)); break;
			case ACLCUR_RET: _aclctx_err(h_ctx, EMIT_aclcur_ret(p, h_ctx)); break;
			case ACLCUR_OP1: _aclctx_err(h_ctx, EMIT_aclcur_op1(p, h_ctx)); break;
			case ACLCUR_OP2: _aclctx_err(h_ctx, EMIT_aclcur_op2(p, h_ctx)); break;
			case ACLCUR_OP3: _aclctx_err(h_ctx, EMIT_aclcur_op3(p, h_ctx)); break;
			case ACLCUR_ST:  _aclctx_err(h_ctx, EMIT_aclcur_st(p, h_ctx)); break;
			case ACLCUR_OPX: _aclctx_err(h_ctx, EMIT_aclcur_opx(p, h_ctx)); break;
					 /** operand links own no emission. */

			case ACLCUR_OPX_MBR: continue;

			case ACLCUR_REG:
					     /** aliases / member views own no storage. */
					     unless(p->m_d.m_reg.m_in.m_emit_reg)
						     continue;

					     aclctx_err_check2(h_ctx, p->m_d.m_reg.m_in.m_emit_reg(p->m_id, h_ctx))
						     return h_ctx->m_err;
					     break;

			case ACLCUR_LOAD: {
						  h_aclcur_t p_reg = aclcur_get_node(p->m_d.m_load.m_reg, h_ctx);
						  aclcur_reg_emit_t* hook;

						  /** nobody reads this value: the call-site
						   * clobber materialises reloads ahead of their
						   * reads, and the unused ones are dead weight.
						   * a volatile load is never elided. */
						  unless(p->m_num_ref
								  || (p->m_flag0 & aclcur_flag0_load(
										  ACLCUR_FLAG0_LOAD_EXPLICIT)))
							  continue;

						  ae2f_ifezerr(p_reg) {
							  _aclctx_err(h_ctx, ACLSPV_COMPILE_UNHANDLED);
							  return h_ctx->m_err;
						  }

						  ae2f_ifnezerr_strict(p_reg->m_class != ACLCUR_REG) {
							  _aclctx_err(h_ctx, ACLSPV_COMPILE_UNHANDLED);
							  return h_ctx->m_err;
						  }

						  /** explicit (volatile) load dispatches to its own hook. */
						  hook = (p->m_flag0 & aclcur_flag0_load(ACLCUR_FLAG0_LOAD_EXPLICIT))
							  ? p_reg->m_d.m_reg.m_in.m_emit_load_explicit
							  : p_reg->m_d.m_reg.m_in.m_emit_load;

						  ae2f_ifezerr(hook) {
							  _aclctx_err(h_ctx, ACLSPV_COMPILE_UNHANDLED);
							  return h_ctx->m_err;
						  }

						  aclctx_err_check2(h_ctx, hook(p->m_id, h_ctx))
							  return h_ctx->m_err;

						  break;
					  }

			case ACLCUR_UNREACHABLE:
				/** a structurally emitted block with no live incoming
				 * edge still needs a terminator; OpUnreachable is the
				 * only valid one that does not add an edge. */
				ae2f_ifezerr(h_ctx->m_count.m_fnimpl = emit_opcode(
							&h_ctx->m_section.m_fnimpl
							, h_ctx->m_count.m_fnimpl
							, SpvOpUnreachable
							, 0))
					return ACLSPV_COMPILE_ALLOC_FAILED_CURSOR;
				break;

			case ACLCUR_STORE: {
						   h_aclcur_t p_reg = aclcur_get_node(p->m_d.m_store.m_reg, h_ctx);
						   aclcur_reg_emit_t* hook;

						   ae2f_ifezerr(p_reg) {
							   _aclctx_err(h_ctx, ACLSPV_COMPILE_UNHANDLED);
							   return h_ctx->m_err;
						   }

						   ae2f_ifnezerr_strict(p_reg->m_class != ACLCUR_REG) {
							   _aclctx_err(h_ctx, ACLSPV_COMPILE_UNHANDLED);
							   return h_ctx->m_err;
						   }

						   /** explicit (volatile) store dispatches to its own hook. */
						   hook = (p->m_flag0 & aclcur_flag0_store(ACLCUR_FLAG0_STORE_EXPLICIT))
							   ? p_reg->m_d.m_reg.m_in.m_emit_store_explicit
							   : p_reg->m_d.m_reg.m_in.m_emit_store;

						   ae2f_ifezerr(hook) {
							   _aclctx_err(h_ctx, ACLSPV_COMPILE_UNHANDLED);
							   return h_ctx->m_err;
						   }

						   aclctx_err_check2(h_ctx, hook(p->m_id, h_ctx))
							   return h_ctx->m_err;
						   break;
					   }

			default:
					   ae2f_unreachable();
		}

#if	__ACLSPV_CHECK_EXTRA
		aclctx_err_check(h_ctx) {
			err_puts("h_ctx->m_err was not 0\n");
			fixme_puts("seeing this means you forgot to make short-circuit.\n");
			return h_ctx->m_err;
		}
#else
		ae2f_assume(!h_ctx->m_err);
#endif

		if(
				(h_ctx->m_usrcfg.m_flag0 & ACLSPV_CTX_FLAG0_SYMBOL)
				&& (p->m_flag0 & aclcur_flag0(ACLCUR_FLAG0_CURSOR_ASSIGNED))
				&& !clang_Cursor_isNull(p->m_cur)) {
			CXString str = clang_getCursorUSR(p->m_cur);
			const char* strbuf = clang_getCString(str);

			ae2f_ifezerr(strbuf) {
				clang_disposeString(str);
				_aclctx_err(h_ctx, ACLSPV_COMPILE_ALLOC_FAILED_CURSOR);
				return h_ctx->m_err;
			}

			/** a register emits its ADDRESS; every other node emits m_id. */
			_aclutil_mkname_dbg(L
					, (p->m_class == ACLCUR_REG && p->m_d.m_reg.m_id_addr)
						? p->m_d.m_reg.m_id_addr : p->m_id
					, strbuf, (*h_ctx), (h_ctx->m_section.m_name));
			clang_disposeString(str);
		}

		aclctx_err_check(h_ctx)
			return h_ctx->m_err;
	}

	return h_ctx->m_err;
}
