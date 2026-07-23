#include "./vardecl.h"

#include <aclmem.h>
#include <aclcursor.h>
#include <util/emitx.h>

#include <.aclspv/util/u32_to_hex8.h>
#include <.aclspv/util/mkname.h>

#include <ae2f/pat/dbg.h>

aclspv_bool_t	ae2f_ccpure aclcursor_is_vardecl(
		const h_aclcursor_t	rd_cursor
		) {
	return rd_cursor->m_kind == ACLCURSOR_VARDECL;
}

CXCursor	aclcursor_get_vardecl_from_ref(
		const CXCursor		c_cur,
		const h_aclspv_ctx_t	h_ctx
		) {
	(void)h_ctx;
	switch((uint_leastenum_t)c_cur.kind) {
		default: return clang_getCursorReferenced(c_cur);
	}
}

enum ACLSPV_COMPILE_ aclcursor_vardecl_evuclbl(
		const aclspv_wrdcount_t c_num_cursor, 
		x_aclcursor* ae2f_restrict const wr_cursor, 
		const h_aclspv_ctx_t	h_ctx,
		uintmax_t	c_cursor_lvl
		) {
	aclspv_wrdcount_t	IDX = c_num_cursor;
	ae2fpat_assertassume_strict(wr_cursor);
	
	ae2f_expected_if(wr_cursor) while(IDX--) {
#define	CURSORDATA	wr_cursor[IDX].m_data.m_vardecl
		unless(aclcursor_is_vardecl(wr_cursor + IDX))
			continue;

		dbg_prefix();
		dbg_call((void), fprintf(stderr, "stagecmp %lu %lu\n"
					, (unsigned long)wr_cursor[IDX].m_data.m_vardecl.m_stage_lvl
					, (unsigned long)c_cursor_lvl));

		if(wr_cursor[IDX].m_data.m_vardecl.m_stage_lvl >= c_cursor_lvl)
			continue;

		if((CURSORDATA.m_mask & (ACLCURSOR_VARDECL_ALLOCATED | ACLCURSOR_VARDECL_OCCUPIED))
				== (ACLCURSOR_VARDECL_ALLOCATED | ACLCURSOR_VARDECL_OCCUPIED)) {
			if(CURSORDATA.m_mask & ACLCURSOR_VARDECL_PREDICTABLE &&
					(CURSORDATA.m_init_val && !ae2f_expected(
										 h_ctx->m_count.m_fnimpl = aclutil_emitx_3(
											 &h_ctx->m_section.m_fnimpl
											 , h_ctx->m_count.m_fnimpl
											 , SpvOpStore
											 , CURSORDATA.m_id
											 , CURSORDATA.m_init_val
											 )
										)
					)) return ACLSPV_COMPILE_ALLOC_FAILED_CURSOR;

			dbg_prefix(); dbg_call((void), fprintf(stderr
						, "tell cursor label %u %u\n"
						, IDX, (unsigned)CURSORDATA.m_mask & ACLCURSOR_VARDECL_PREDICTABLE
						));

			CURSORDATA.m_mask &= ~(aclspv_wrd_t)ACLCURSOR_VARDECL_PREDICTABLE;
		}
		else if(CURSORDATA.m_mask & ACLCURSOR_VARDECL_OWNER) {
			CURSORDATA.m_mask |= ACLCURSOR_VARDECL_ALLOCATED;
			CURSORDATA.m_mask &= ~(aclspv_wrd_t)(ACLCURSOR_VARDECL_PREDICTABLE);

			ae2f_expected_but_else(h_ctx->m_count.m_fndef = util_emitx_variable(
						&h_ctx->m_section.m_fndef
						, h_ctx->m_count.m_fndef
						, CURSORDATA.m_ptr_type_id
						, CURSORDATA.m_id
						, SpvStorageClassFunction))
				return ACLSPV_COMPILE_ALLOC_FAILED_CURSOR;

			if(CURSORDATA.m_init_val && !ae2f_expected(
						h_ctx->m_count.m_fnimpl = aclutil_emitx_3(
							&h_ctx->m_section.m_fnimpl
							, h_ctx->m_count.m_fnimpl
							, SpvOpStore
							, CURSORDATA.m_id
							, CURSORDATA.m_init_val
							)
						)
			  ) return ACLSPV_COMPILE_ALLOC_FAILED_CURSOR;

			if(h_ctx->m_usrcfg.m_flag0 & ACLSPV_CTX_FLAG0_SYMBOL)
			{
				CXString SPELL	= clang_getCursorSpelling(wr_cursor[IDX].m_cursor);


				aclutil_mkname_dbg_fn(
						CURSORDATA.m_id
						, SPELL.data
						, h_ctx
						, &h_ctx->m_tmp.m_v0
						);

				clang_disposeString(SPELL);
				if(h_ctx->m_err) return h_ctx->m_err;
			}

		} else {

		}
#undef	CURSORDATA
	}

	return ACLSPV_COMPILE_OK;
}

aclspv_wrdcount_t aclcursor_vardecl_find_stagedval(
		const aclspv_wrdcount_t	c_num_cursor,
		const x_aclspv_vec* const ae2f_restrict h_cursors,
		const umax c_stage, 
		const aclspv_id_t c_ty
		) {
#define	CURSORS	((const x_aclcursor* ae2f_restrict)h_cursors->m_p)

	aclspv_wrdcount_t	CURI = c_num_cursor;

	ae2fpat_assertassume(h_cursors);
	ae2fpat_assertassume(CURSORS);

	while(CURI--) {
		const x_aclcursor C = CURSORS[CURI];
		if(clang_getCursorKind(C.m_cursor) != CXCursor_VarDecl)
			continue;

		if(C.m_data.m_vardecl.m_type_id ^ c_ty)
			continue;

		if(C.m_data.m_vardecl.m_stage_lvl <= c_stage)
			continue;

		if(C.m_data.m_vardecl.m_mask & ACLCURSOR_VARDECL_OCCUPIED)
			continue;

		return CURI;
	}

	return 0xFFFFFFFF;
}
