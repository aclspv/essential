/** @file cursor.c */

#include <ae2f/bll.h>
#include <clang-c/Index.h>
#include <util/cursor.h>
#include <assert.h>
#include <spirv/unified1/spirv.h>
#include "./ctx.h"
#include "./emitx.h"
#include <.aclspv/util/u32_to_hex8.h>
#include "aclspv.h"
#include "util/log.h"


aclspv_wrdcount_t aclutil_find_cursor(
		const aclspv_wrdcount_t			c_num_cursor, 
		const util_cursor* ae2f_restrict const	rd_cursors,
		const CXCursor				c_cursor_to_find
		)
{
	aclspv_wrdcount_t	IDX = c_num_cursor;
	ae2f_expected_but_else(rd_cursors)	return c_num_cursor;
	while(IDX-- && !clang_equalCursors(rd_cursors[IDX].m_cursor, c_cursor_to_find)) {}
	return ae2f_bll_sel_bool1(aclspv_wrdcount_t, IDX < c_num_cursor, IDX, c_num_cursor);
}

aclspv_wrdcount_t aclutil_mk_cursor_base(
		const aclspv_wrdcount_t			c_num_cursor,
		x_aclspv_vec* const ae2f_restrict	h_cursors,
		const CXCursor				c_cursor_to_add
		)
{
	CXCursor		CUR_DUMMY;
	const aclspv_wrdcount_t IDX 
		= aclutil_find_cursor(
				c_num_cursor
				, h_cursors->m_p
				, c_cursor_to_add
				);

	{ ae2f_assume((size_t)(c_num_cursor * sizeof(util_cursor)) >= h_cursors->m_sz); }
	unless(IDX == c_num_cursor) return IDX;

	_aclspv_grow_vec_with_copy(
			_aclspv_malloc
			, _aclspv_free
			, _aclspv_memcpy
			, L_new
			, h_cursors[0]
			, ((size_t)((c_num_cursor + 1) * sizeof(util_cursor)))
			);

	*ae2f_bll_sel_ptrbool1(
			CXCursor* ae2f_restrict
			, !!h_cursors->m_p
			, &((util_cursor* ae2f_restrict)h_cursors->m_p)[c_num_cursor].m_cursor
			, &CUR_DUMMY) = c_cursor_to_add;

	return c_num_cursor + (!h_cursors->m_p);
}

enum ACLSPV_COMPILE_ aclutil_tell_cursor_lbl(
		const aclspv_wrdcount_t c_num_cursor, 
		util_cursor* ae2f_restrict const wr_cursor, 
		const h_util_ctx_t		h_ctx,
		uintmax_t			c_cursor_lvl
		) {
	aclspv_wrdcount_t	IDX = c_num_cursor;
	assert(wr_cursor || !c_num_cursor);
	{ ae2f_assume(wr_cursor || !c_num_cursor); }
	ae2f_expected_if(wr_cursor) while(IDX--) {
#define	CURSORDATA	wr_cursor[IDX].m_data.m_vardecl
		unless(wr_cursor[IDX].m_cursor.kind == CXCursor_VarDecl)
			continue;

		dbg_prefix();
		dbg_call((void), fprintf(stderr, "stagecmp %lu %lu\n"
					, (unsigned long)wr_cursor[IDX].m_data.m_vardecl.m_stage_lvl
					, (unsigned long)c_cursor_lvl));

		if(wr_cursor[IDX].m_data.m_vardecl.m_stage_lvl >= c_cursor_lvl)
			continue;

		if((CURSORDATA.m_mask & (ACLUTIL_CURSOR_VAR_ALLOCATED | ACLUTIL_CURSOR_VAR_OCCUPIED)) 
				== (ACLUTIL_CURSOR_VAR_ALLOCATED | ACLUTIL_CURSOR_VAR_OCCUPIED)) {
			if(CURSORDATA.m_mask & ACLUTIL_CURSOR_VAR_PREDICTABLE &&
					(CURSORDATA.m_init_val && !ae2f_expected(
										 h_ctx->m_count.m_fnimpl = util_emitx_3(
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
						, IDX, (unsigned)CURSORDATA.m_mask & ACLUTIL_CURSOR_VAR_PREDICTABLE
						));

			CURSORDATA.m_mask &= ~(aclspv_wrd_t)ACLUTIL_CURSOR_VAR_PREDICTABLE;
		}
		else if(CURSORDATA.m_mask & ACLUTIL_CURSOR_VAR_OWNER) {
			CURSORDATA.m_mask |= ACLUTIL_CURSOR_VAR_ALLOCATED;
			CURSORDATA.m_mask &= ~(aclspv_wrd_t)(ACLUTIL_CURSOR_VAR_PREDICTABLE);

			ae2f_expected_but_else(h_ctx->m_count.m_fndef = util_emitx_variable(
						&h_ctx->m_section.m_fndef
						, h_ctx->m_count.m_fndef
						, CURSORDATA.m_ptr_type_id
						, CURSORDATA.m_id
						, SpvStorageClassFunction))
				return ACLSPV_COMPILE_ALLOC_FAILED_CURSOR;

			if(CURSORDATA.m_init_val && !ae2f_expected(
						h_ctx->m_count.m_fnimpl = util_emitx_3(
							&h_ctx->m_section.m_fnimpl
							, h_ctx->m_count.m_fnimpl
							, SpvOpStore
							, CURSORDATA.m_id
							, CURSORDATA.m_init_val
							)
						)
			  ) return ACLSPV_COMPILE_ALLOC_FAILED_CURSOR;

			if(h_ctx->m_flag0 & ACLSPV_CTX_FLAG0_SYMBOL)
			{
				const aclspv_wrd_t POS	= h_ctx->m_count.m_name;
				CXString SPELL		= clang_getCursorSpelling(wr_cursor[IDX].m_cursor);

				_aclspv_grow_vec(_aclspv_malloc, _aclspv_free	
						, h_ctx->m_tmp.m_v0, strlen(SPELL.data) + 10
						);
				ae2f_expected_but_else(h_ctx->m_tmp.m_v0.m_p)
					goto LBL_FAIL;
				((char* ae2f_restrict)h_ctx->m_tmp.m_v0.m_p)[8] = ':';
				((char* ae2f_restrict)h_ctx->m_tmp.m_v0.m_p)[9] = '\0';
				_util_u32_to_hex8((CURSORDATA.m_id)
						, ((char* ae2f_restrict)h_ctx->m_tmp.m_v0.m_p));
				strcat(h_ctx->m_tmp.m_v0.m_p, SPELL.data);
				ae2f_expected_but_else((h_ctx->m_count.m_name = emit_opcode(
								&h_ctx->m_section.m_name
								, h_ctx->m_count.m_name
								, SpvOpName, 0))
						) goto LBL_FAIL;					
				ae2f_expected_but_else((h_ctx->m_count.m_name = util_emit_wrd(
								&h_ctx->m_section.m_name
								, h_ctx->m_count.m_name
								, CURSORDATA.m_id
								)))						
					goto LBL_FAIL;								
				ae2f_expected_but_else((h_ctx->m_count.m_name = util_emit_str(
								&h_ctx->m_section.m_name			
								, h_ctx->m_count.m_name		
								, h_ctx->m_tmp.m_v0.m_p 			
								)))						
					goto LBL_FAIL;								
				set_oprnd_count_for_opcode(get_wrd_of_vec(					
							&h_ctx->m_section.m_name)[POS]				
						, h_ctx->m_count.m_name - POS - 1);

				clang_disposeString(SPELL);
				continue;
				ae2f_unreachable();
LBL_FAIL:
				clang_disposeString(SPELL);
				return ACLSPV_COMPILE_ALLOC_FAILED_CURSOR;
			}

		} else {

		}
#undef	CURSORDATA
	}

	return ACLSPV_COMPILE_OK;
}

#include <ae2f/pat/dbg.h>

aclspv_wrdcount_t aclutil_from_cursor_find_stagedval(
		const aclspv_wrdcount_t		c_num_cursor,
		const x_aclspv_vec* const ae2f_restrict h_cursors,
		const umax c_stage, 
		const aclspv_id_t c_ty
		) {
#define	CURSORS	((const util_cursor* ae2f_restrict)h_cursors->m_p)

	aclspv_wrdcount_t	CURI = c_num_cursor;

	ae2fpat_assertassume(h_cursors);
	ae2fpat_assertassume(CURSORS);

	while(CURI--) {
		const util_cursor C = CURSORS[CURI];
		if(clang_getCursorKind(C.m_cursor) != CXCursor_VarDecl)
			continue;

		if(C.m_data.m_vardecl.m_type_id ^ c_ty)
			continue;

		if(C.m_data.m_vardecl.m_stage_lvl <= c_stage)
			continue;

		if(C.m_data.m_vardecl.m_mask & ACLUTIL_CURSOR_VAR_OCCUPIED)
			continue;

		return CURI;
	}

	return 0xFFFFFFFF;
}
