#include "./emit.h"
#include "./util.h"
#include "./flag0.h"
#include "./phi.h"
#include "./j/emit.h"
#include "./jc/emit.h"
#include "./ret/emit.h"
#include "./op/emit.h"
#include "./opX/emit.h"

#include <.aclspv/util/mkname.h>

aclspv_compile_t aclcur_emit(h_aclspv_ctx_t h_ctx) {
	aclspv_wrd_t i;

	/** assumable. it should be internal. */
	ae2f_assume(h_ctx);

	ae2f_ifnezerr(h_ctx->m_err)
		return h_ctx->m_err;

	for(i = 0; i < h_ctx->m_num_cursor; ++i) {
		h_aclcur_t p = &s_get_buf(h_ctx)[i];

		/** if emitting none, continue. otherwise, break the switch. */
		switch(p->m_class) {
			case ACLCUR_RESERVED:
				/** phis are emitted through their owning LOAD node. */
			case ACLCUR_PHI:
				continue;

			case ACLCUR_LBL:
				{
					const aclspv_wrd_t c_blk = p->m_d.m_lbl.m_nop;

					ae2f_ifnezerr(h_ctx->m_err = EMIT_aclcur_lbl(p, h_ctx))
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
								ae2f_ifnezerr(h_ctx->m_err 
										= p_mid->m_d.m_reg.m_in.m_emit_reg(p_mid->m_id, h_ctx)
									     ) return h_ctx->m_err;
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

							ae2f_ifnezerr(h_ctx->m_err = aclcur_phi_emit(head, h_ctx))
								return h_ctx->m_err;

							if(ne < 64)
								emd[ne++] = head;
						}
					}

					break;
				}
			case ACLCUR_J:   h_ctx->m_err = EMIT_aclcur_jmp(p, h_ctx); break;
			case ACLCUR_JC:  h_ctx->m_err = EMIT_aclcur_jc(p, h_ctx); break;
			case ACLCUR_RET: h_ctx->m_err = EMIT_aclcur_ret(p, h_ctx); break;
			case ACLCUR_OP1: h_ctx->m_err = EMIT_aclcur_op1(p, h_ctx); break;
			case ACLCUR_OP2: h_ctx->m_err = EMIT_aclcur_op2(p, h_ctx); break;
			case ACLCUR_OP3: h_ctx->m_err = EMIT_aclcur_op3(p, h_ctx); break;
			case ACLCUR_ST:  h_ctx->m_err = EMIT_aclcur_st(p, h_ctx); break;
			case ACLCUR_OPX: h_ctx->m_err = EMIT_aclcur_opx(p, h_ctx); break;
					 /** operand links own no emission. */

			case ACLCUR_OPX_MBR: continue;

			case ACLCUR_REG:
					     /** aliases / member views own no storage. */
					     unless(p->m_d.m_reg.m_in.m_emit_reg)
						     continue;

					     ae2f_ifnezerr(h_ctx->m_err = p->m_d.m_reg.m_in.m_emit_reg(p->m_id, h_ctx))
						     return h_ctx->m_err;
					     break;

			case ACLCUR_LOAD: {
						  h_aclcur_t p_reg = aclcur_get_node(p->m_d.m_load.m_reg, h_ctx);
						  aclcur_reg_emit_t* hook;

						  ae2f_ifezerr(p_reg)
							  return h_ctx->m_err = ACLSPV_COMPILE_UNHANDLED;

						  ae2f_ifnezerr_strict(p_reg->m_class != ACLCUR_REG)
							  return h_ctx->m_err = ACLSPV_COMPILE_UNHANDLED;

						  /** explicit (volatile) load dispatches to its own hook. */
						  hook = (p->m_flag0 & aclcur_flag0_load(ACLCUR_FLAG0_LOAD_EXPLICIT))
							  ? p_reg->m_d.m_reg.m_in.m_emit_load_explicit
							  : p_reg->m_d.m_reg.m_in.m_emit_load;

						  unless(hook)
							  return h_ctx->m_err = ACLSPV_COMPILE_UNHANDLED;

						  ae2f_ifnezerr(h_ctx->m_err = hook(p->m_id, h_ctx))
							  return h_ctx->m_err;

						  break;
					  }

			case ACLCUR_STORE: {
						   h_aclcur_t p_reg = aclcur_get_node(p->m_d.m_store.m_reg, h_ctx);
						   aclcur_reg_emit_t* hook;

						   ae2f_ifezerr(p_reg)
							   return h_ctx->m_err = ACLSPV_COMPILE_UNHANDLED;

						   ae2f_ifnezerr_strict(p_reg->m_class != ACLCUR_REG)
							   return h_ctx->m_err = ACLSPV_COMPILE_UNHANDLED;

						   /** explicit (volatile) store dispatches to its own hook. */
						   hook = (p->m_flag0 & aclcur_flag0_store(ACLCUR_FLAG0_STORE_EXPLICIT))
							   ? p_reg->m_d.m_reg.m_in.m_emit_store_explicit
							   : p_reg->m_d.m_reg.m_in.m_emit_store;

						   unless(hook)
							   return h_ctx->m_err = ACLSPV_COMPILE_UNHANDLED;

						   ae2f_ifnezerr(h_ctx->m_err = hook(p->m_id, h_ctx))
							   return h_ctx->m_err;
						   break;
					   }

			default:
					   ae2f_unreachable();
		}

#if	__ACLSPV_CHECK_EXTRA
		ae2f_ifnezerr(h_ctx->m_err) {
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
				return h_ctx->m_err = ACLSPV_COMPILE_ALLOC_FAILED_CURSOR;
			}

			_aclutil_mkname_dbg(L, p->m_id, strbuf, (*h_ctx), (h_ctx->m_section.m_name));
			clang_disposeString(str);
		}

		ae2f_ifnezerr(h_ctx->m_err)
			return h_ctx->m_err;
	}

	return h_ctx->m_err;
}
