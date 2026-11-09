#include "./op.h"
#include "./load/get.h"
#include "./util.h"
#include "./common.h"

#include <ae2f/cc/pause.h>
#include <ae2f/cc/assume.h>

#include <aclutil/emitx.h>

/** create an opx node from operand words that are already resolved.
 *
 * `c_num_val` leading operands are value ids and carry a reference; the
 * remaining words are raw operands (e.g. OpVectorShuffle components) that
 * are emitted verbatim and referenced by nothing. */
static aclspv_id_t s_cfg_opx_build(
		aclspv_id_reserved_t	c_newid_opt,
		enum SpvOp_	c_op,
		aclty_id_t	c_ty,
		aclspv_id_t	c_set,
		aclspv_wrd_t	c_inst,
		const aclspv_vid_t	c_os[],
		aclspv_wrd_t	c_num,
		aclspv_wrd_t	c_num_val,
		h_aclspv_ctx_t	h_ctx
		) {
	aclspv_wrd_t i;

	unless(c_num) {
#if	__ACLSPV_CHECK_EXTRA
		err_puts("opX needs at least 1 operand.");
#endif
		return 0;
	}

	/** opX holds at most ACLCUR_OPX_OPRND_CAP operands (fixed-size p_os array). */
	if(c_num > __ACLSPV_MAX_OPRND) {
#if	__ACLSPV_CHECK_EXTRA
		err_puts("opX holds at most 256 operands.");
#endif
		return 0;
	}

	/** all operands are order-sensitive: no swap here. */
	{
		const aclcur_t* ae2f_restrict buf = s_get_buf_c(h_ctx);
		aclspv_wrd_t k;

		for(k = h_ctx->m_num_cursor; k --> 0; ae2fcc_pause()) {
			if(
					buf[k].m_class == ACLCUR_OPX
					&& buf[k].m_d.m_opx.m_op	== c_op
					&& buf[k].m_ty			== c_ty
					&& buf[k].m_d.m_opx.m_num	== c_num
					&& buf[k].m_d.m_opx.m_set	== c_set
					&& buf[k].m_d.m_opx.m_inst	== c_inst
			  ) {
				/** compare the operand chain in order. */
				aclspv_wrd_t idx = buf[k].m_d.m_opx.m_o_head;
				aclspv_wrd_t j = 0;

				while(idx && j < c_num) {
					const aclcur_t* ae2f_restrict p_mbr = &buf[idx];
#if	__ACLSPV_CHECK_EXTRA
					if(p_mbr->m_class != ACLCUR_OPX_MBR) break;
#endif
					if(p_mbr->m_d.m_opx_mbr.m_operand != c_os[j]) break;
					idx = p_mbr->m_d.m_opx_mbr.m_nxt;
					++j;
				}

				/** full match: same count, no early terminator. */
				if(j == c_num && !idx)
					return buf[k].m_id;
			}
		}
	}

	/** need new */
	{
		aclspv_id_t opx = aclcur_cfg_node(c_newid_opt, h_ctx);
		h_aclcur_t p_opx = aclcur_get_node(opx, h_ctx);
		aclspv_id_t mbr_prv = 0;

		ae2f_ifezerr(p_opx) return 0;

		p_opx->m_class = ACLCUR_OPX;
		p_opx->m_val = p_opx->m_id;
		p_opx->m_flag0 = aclcur_flag0(ACLCUR_FLAG0_VALUE);
		p_opx->m_ty = c_ty;
		p_opx->m_d.m_opx.m_op = c_op;
		p_opx->m_d.m_opx.m_num = c_num;
		p_opx->m_d.m_opx.m_o_head = 0;
		p_opx->m_d.m_opx.m_set = c_set;
		p_opx->m_d.m_opx.m_inst = c_inst;

		/** operand links: chained via cursor-buffer index, skipped in the instruction stream. */
		for(i = 0; i < c_num; ++i) {
			aclspv_id_t mbr;
			h_aclcur_t p_mbr;
			aclspv_wrd_t idx;

			mbr = aclcur_cfg_node2(0, h_ctx, 1);

			ae2f_ifezerr(mbr) {
				/** link allocation failed: make the opx node inert. */
				p_opx = aclcur_get_node(opx, h_ctx);
				ae2f_ifezerr(p_opx) return 0;
				p_opx->m_d.m_opx.m_num = 0;
				p_opx->m_d.m_opx.m_o_head = 0;
				return 0;
			}

			/** re-fetch after cfg_node2: the cursor buffer may have been reallocated. */
			p_opx = aclcur_get_node(opx, h_ctx);
			p_mbr = aclcur_get_node(mbr, h_ctx);

			ae2f_ifezerr(p_opx) return 0;
			ae2f_ifezerr(p_mbr) return 0;

			p_mbr->m_class = ACLCUR_OPX_MBR;
			p_mbr->m_d.m_opx_mbr.m_operand = c_os[i];
			p_mbr->m_d.m_opx_mbr.m_nxt = 0;

			idx = (aclspv_wrd_t)(p_mbr - s_get_buf(h_ctx));

			if(i == 0)
				p_opx->m_d.m_opx.m_o_head = idx;
			else {
				h_aclcur_t p_prv = aclcur_get_node(mbr_prv, h_ctx);

				ae2f_ifezerr(p_prv) return 0;
				p_prv->m_d.m_opx_mbr.m_nxt = idx;
			}

			mbr_prv = mbr;
		}

		/** member links may have regrown (realloc'd) the buffer: re-resolve.
		 * only the value operands carry a reference. */
		for(i = 0; i < c_num_val; ++i) {
			h_aclcur_t p_os_i = aclcur_get_node(c_os[i], h_ctx);
			if(p_os_i)	(void)++p_os_i->m_num_ref;
		}

		return p_opx->m_id;
	}

	ae2f_unreachable();
}

static aclspv_id_t s_cfg_opx(
		aclspv_id_reserved_t	c_newid_opt,
		enum SpvOp_	c_op,
		aclty_id_t	c_ty,
		aclspv_id_t	c_set,
		aclspv_wrd_t	c_inst,
		const aclspv_vid_t	c_os[],
		aclspv_wrd_t	c_num,
		h_aclspv_ctx_t	h_ctx
		) {
	aclcur_t b;
	aclspv_wrd_t i;

	/** every operand must resolve to a value. */
	for(i = 0; i < c_num; ++i) {
		b = aclcur_get_load(c_os[i], h_ctx);
		ae2f_ifezerr(b.m_val) return 0;

		/** constant operands are permitted: their ids are emitted as raw operands. */
#if	__ACLSPV_CHECK_EXTRA
		ae2f_ifnezerr(b.m_id && !aclcur_get_node(b.m_id, h_ctx)) {
			err_puts(
					"operand node missing on aclcur opx.\n"
					"it should be handled on aclemit.\n"
					);
			h_ctx->m_err = ACLSPV_COMPILE_UNHANDLED;
			return 0;
		}
#else
		ae2f_assume_strict(!b.m_id || aclcur_get_node(b.m_id, h_ctx));
#endif
	}

	return s_cfg_opx_build(
			c_newid_opt, c_op, c_ty, c_set, c_inst
			, c_os, c_num, c_num, h_ctx);
}

aclspv_id_t aclcur_cfg_opx(
		aclspv_id_reserved_t	c_newid_opt,
		enum SpvOp_	c_op,
		aclty_id_t	c_ty,
		const aclspv_vid_t	c_os[],
		aclspv_wrd_t	c_num,
		h_aclspv_ctx_t	h_ctx
		) {
	return s_cfg_opx(c_newid_opt, c_op, c_ty, 0, 0, c_os, c_num, h_ctx);
}

aclspv_id_t aclcur_cfg_extinst(
		aclspv_id_reserved_t	c_newid_opt,
		aclty_id_t		c_ty,
		aclspv_id_t		c_set,
		aclspv_wrd_t		c_inst,
		const aclspv_vid_t	c_os[],
		aclspv_wrd_t		c_num,
		h_aclspv_ctx_t		h_ctx
		) {
	return s_cfg_opx(c_newid_opt, SpvOpExtInst, c_ty, c_set, c_inst, c_os, c_num, h_ctx);
}

aclspv_id_t aclcur_cfg_shuffle(
		aclspv_id_reserved_t	c_newid_opt,
		aclty_id_t		c_ty,
		aclspv_id_t		c_v1,
		aclspv_id_t		c_v2,
		const aclspv_wrd_t*	c_comps,
		aclspv_wrd_t		c_num,
		h_aclspv_ctx_t		h_ctx
		) {
	aclspv_wrd_t		os[__ACLSPV_MAX_OPRND];
	aclcur_t		b;
	aclspv_wrd_t		i;

	ae2f_ifezerr(c_num) return 0;

	if((aclspv_wrd_t)2 + c_num > __ACLSPV_MAX_OPRND)
		return 0;

	/** the two vectors are ordinary value operands. */
	b = aclcur_get_load(c_v1, h_ctx);
	ae2f_ifezerr(b.m_val) return 0;
	os[0] = c_v1;

	b = aclcur_get_load(c_v2, h_ctx);
	ae2f_ifezerr(b.m_val) return 0;
	os[1] = c_v2;

	/** components are literal indices into v1 ++ v2, emitted verbatim. */
	for(i = 0; i < c_num; ++i)
		os[2 + i] = c_comps[i];

	return s_cfg_opx_build(
			c_newid_opt
			, SpvOpVectorShuffle
			, c_ty
			, 0, 0
			, os
			, (aclspv_wrd_t)2 + c_num
			, 2
			, h_ctx);
}
