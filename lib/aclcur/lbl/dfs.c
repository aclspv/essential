#include "./dfs.h"
#include "../lbl.h"
#include "../util.h"

#include <aclmem.h>
#include <util/log.h>

/** current | parent */
typedef aclspv_wrd2_t s_visit_t;

ae2f_inline_f ae2f_ccpure static s_visit_t* s_get_visit(h_aclspv_ctx_t h_ctx) {
	return h_ctx->m_cur_lbl_dfs_visit.m_p;
}

ae2f_inline_f ae2f_ccpure static s_visit_t* s_get_stack(h_aclspv_ctx_t h_ctx) {
	return h_ctx->m_cur_lbl_dfs.m_p;
}

ae2f_inline_f ae2f_ccpure static aclspv_bool_t s_visit_contains(
		s_visit_t c,
		h_aclspv_ctx_t h_ctx,
		aclspv_wrd_t c_num_visit
		) {

	while(c_num_visit-->0) {
		if(s_get_visit(h_ctx)[c_num_visit] == c)
			return 1;
	}

	return 0;
}

static enum ACLCUR_LBL_DFS_ s_visit(
		enum ACLCUR_LBL_DFS_	c_code,
		aclcur_lbl_dfs_t fn,
		void* rdwr_client,
		h_aclspv_ctx_t	h_ctx,
		/****************************/
		aclspv_wrd_t	c_num_stack,
		aclspv_wrd_t	c_num_visit
		) {
	s_visit_t	elm;

TAILJMP:
	unless(c_num_stack)			return c_code;

	/** break the loop */
	if(c_code == ACLCUR_LBL_DFS_BREAK)	return c_code;

	elm = s_get_stack(h_ctx)[--c_num_stack];

	if(s_visit_contains(elm, h_ctx, c_num_visit)) {
		goto TAILJMP;
	} else {
		h_aclcur_t p_cur = aclcur_get_node((aclspv_wrd_t)(elm & 0xFFFFFFFF), h_ctx);
		h_aclcur_t p_cur_tail; 
		h_aclcur_t p_par = aclcur_get_node((aclspv_wrd_t)(elm >> 32u), h_ctx);

		ae2f_ifnezerr_strict(s_aclmem_cgrow2(
					&h_ctx->m_cur_lbl_dfs_visit
					, ++c_num_visit
					, sizeof(s_visit_t)
					, ACLSPV_COMPILE_ALLOC_FAILED))
			return c_code;

		s_get_visit(h_ctx)[c_num_visit - 1] = elm;
		ae2f_ifezerr(p_cur) { 
			c_code = ACLCUR_LBL_DFS_CONTINUE;
			goto TAILJMP;
		}

		switch(fn(p_cur, p_par, rdwr_client)) {
			default:
				err_puts("invalid code on function");
				h_ctx->m_err = ACLSPV_COMPILE_UNHANDLED;
				return c_code;

			case ACLCUR_LBL_DFS_BREAK:
				return ACLCUR_LBL_DFS_BREAK;

			case ACLCUR_LBL_DFS_IN:
				ae2f_ifnezerr_strict(p_cur->m_class != ACLCUR_LBL)
					return c_code;

				p_cur_tail = &p_cur[p_cur->m_d.m_lbl.m_nop - 1];

				ae2f_ifnezerr(p_cur_tail == p_cur)
					goto TAILJMP;

				switch(p_cur_tail->m_class) {
					default:
						break;

					case ACLCUR_J:
						ae2f_ifnezerr_strict(s_aclmem_cgrow2(
									&h_ctx->m_cur_lbl_dfs
									, ++c_num_stack
									, sizeof(s_visit_t)
									, ACLSPV_COMPILE_ALLOC_FAILED))
							return ACLCUR_LBL_DFS_IN;

						s_get_stack(h_ctx)[c_num_stack - 1]
							= ((aclspv_wrd2_t)p_cur_tail->m_d.m_j.m_lbl) | (elm << 32u);

						break;

					case ACLCUR_JC: {
						/** merge / continue targets are successors of the block as well. */
						const aclspv_wrd_t c_ext =
								(aclspv_wrd_t)(p_cur_tail->m_d.m_jc.m_merge != 0)
								+ (aclspv_wrd_t)(p_cur_tail->m_d.m_jc.m_continue != 0);

						ae2f_ifnezerr_strict(s_aclmem_cgrow2(
									&h_ctx->m_cur_lbl_dfs
									, c_num_stack += 2 + c_ext
									, sizeof(s_visit_t)
									, ACLSPV_COMPILE_ALLOC_FAILED))
							return ACLCUR_LBL_DFS_IN;

						s_get_stack(h_ctx)[c_num_stack - 1]
							= ((aclspv_wrd2_t)p_cur_tail->m_d.m_jc.m_lbl_false) | (elm << 32u);

						s_get_stack(h_ctx)[c_num_stack - 2]
							= ((aclspv_wrd2_t)p_cur_tail->m_d.m_jc.m_lbl_true) | (elm << 32u);

						if(p_cur_tail->m_d.m_jc.m_continue)
							s_get_stack(h_ctx)[c_num_stack - 3]
								= ((aclspv_wrd2_t)p_cur_tail->m_d.m_jc.m_continue) | (elm << 32u);
						if(p_cur_tail->m_d.m_jc.m_merge)
							s_get_stack(h_ctx)[c_num_stack - 3 - (p_cur_tail->m_d.m_jc.m_continue != 0)]
								= ((aclspv_wrd2_t)p_cur_tail->m_d.m_jc.m_merge) | (elm << 32u);

						break;
					}
				}

				c_code = ACLCUR_LBL_DFS_IN;
				goto TAILJMP;

			case ACLCUR_LBL_DFS_CONTINUE:
				c_code = ACLCUR_LBL_DFS_CONTINUE;
				goto TAILJMP;
		}

		ae2f_unreachable();
	}
}

enum ACLCUR_LBL_DFS_ aclcur_lbl_dfs(
		aclspv_id_t		c_lbl,
		aclcur_lbl_dfs_t	fn,
		void*			rdwr_client,
		h_aclspv_ctx_t		h_ctx
		) {
	ae2f_ifnezerr_strict(s_aclmem_calloc2(
				&h_ctx->m_cur_lbl_dfs
				, 1 
				, sizeof(s_visit_t)
				, ACLSPV_COMPILE_ALLOC_FAILED))
		return ACLCUR_LBL_DFS_BREAK;

	s_get_stack(h_ctx)[0] = c_lbl;

	return s_visit(
			ACLCUR_LBL_DFS_IN
			, fn, rdwr_client
			, h_ctx
			, 1
			, 0
		      );
}
