/** 
 * @file	simple1.h
 * @brief	int & float literals, variable references
 * */

#ifndef aclexpr_visit_simple1_h
#define aclexpr_visit_simple1_h

#include <util/cursor.h>
#include <util/emitx.h>
#include <util/constant.h>
#include <emit/expr.h>

/** 
 * binary expression with 1 operators.
 * */
ae2f_inline static enum EMIT_EXPR_ s_simple(
		const CXCursor			c_cur,
		const h_util_ctx_t		h_ctx,
		aclspv_id_t* ae2f_restrict const	wr_newid,
		aclspv_id_t* ae2f_restrict const	wr_type_fetched,
		util_literal* ae2f_restrict const	rwr_literal_opt
		) {
	emit_expr_literal EVRES;
	ae2fpat_assertassume(wr_newid);
	ae2fpat_assertassume(h_ctx);

	switch((uint_leastenum_t)c_cur.kind) {
		/** unknown */
		default:
			dbg_prefix(a);
			dbg_call((void), (fprintf(stderr
							, "current cursor kind is not known by simple(1) %u\n"
							, c_cur.kind
						 )));
			return EMIT_EXPR_NOT_THE_CASE;

			/** literals */

		case CXCursor_IntegerLiteral:
		case CXCursor_FloatingLiteral:
			{
				CXEvalResult EVAL = clang_Cursor_Evaluate(c_cur);

				/** no cleanup needed. init already fucked. */
				ae2f_expected_but_else (EVAL) {
					assert(0 && "EVAL is not running");
					return EMIT_EXPR_FAILURE;
				}

				switch((uint_leastenum_t)clang_EvalResult_getKind(EVAL)) {
					case CXEval_Int:
						dbg_puts(("Literal: intmax"));
						EVRES.m_api_intmax = clang_EvalResult_getAsLongLong(EVAL);
						clang_EvalResult_dispose(EVAL);

						dbg_prefix(i);
						dbg_call(fprintf, (stderr
									, "Value fetched: %lu\n"
									, EVRES.m_api_uintmax
								  )
							);

						*wr_type_fetched = (EVRES.m_api_uintmax <= 0xFFFFFFFF 
								? ID_DEFAULT_I32 : ID_DEFAULT_I64);

						if(rwr_literal_opt) *rwr_literal_opt = EVRES;

						ae2fpat_assertassume_strict(*wr_type_fetched);

						ae2f_expected_but_else(aclid_mk_default_id(
									*wr_type_fetched
									, h_ctx)) {
							err_prefix(a);
							err_call(fprintf, (stderr
										, "Default id fetch has failed. %u\n"
										, *wr_type_fetched
									  )
								);
							return EMIT_EXPR_FAILURE;
						}

						switch(aclid_get_bit_width(*wr_type_fetched)) {
							case 32:
								dbg_puts("I32");
								ae2f_expected_but_else(*wr_newid = aclutil_mk_constant_val_id(
											(aclspv_wrd_t)EVRES.m_u64
											, h_ctx
											))
									return EMIT_EXPR_FAILURE;

								return EMIT_EXPR_SUCCESS_LITERAL;

							case 64:
								dbg_puts("I64");
								ae2f_expected_but_else(*wr_newid = aclutil_mk_constant_val64_id(
											(aclspv_wrd_t)EVRES.m_u64
											, h_ctx
											))
									return EMIT_EXPR_FAILURE;

								return EMIT_EXPR_SUCCESS_LITERAL;
							default:
								assert(0);
								ae2f_unreachable();
						}

					case CXEval_Float:
						dbg_puts(("Literal: double (float casted)"));
						EVRES.m_flt = (float)clang_EvalResult_getAsDouble(EVAL);
						dbg_prefix(a);
						dbg_call(fprintf, (stderr, "Literal fetched: %f\n", EVRES.m_flt));

						clang_EvalResult_dispose(EVAL);

						*wr_type_fetched = ID_DEFAULT_F32;

						ae2fpat_assertassume_strict(*wr_type_fetched);

						*rwr_literal_opt = EVRES;

						ae2f_expected_but_else(*wr_newid = aclutil_mk_constant_val_f32_id(
									EVRES.m_flt
									, h_ctx
									))
							return EMIT_EXPR_FAILURE;

						return EMIT_EXPR_SUCCESS_LITERAL;

					default:
						assert(0 && "unknown evaluation");
						return EMIT_EXPR_FAILURE;
				}
			} assert(0); ae2f_unreachable();

		case CXCursor_DeclRefExpr:
			{
				const CXCursor DECL = clang_getCursorReferenced(c_cur);
				const aclspv_wrd_t	DECL_IDX = aclutil_find_cursor(
						h_ctx->m_num_cursor
						, h_ctx->m_cursors.m_p
						, DECL
						);

				ae2f_expected_but_else(DECL_IDX < h_ctx->m_num_cursor)
					return EMIT_EXPR_FAILURE;

#define	DECL_INFO	DECL_IDX[((util_cursor* ae2f_restrict)h_ctx->m_cursors.m_p)]

				/** NaN is undefined behaviour here */
				unless(aclid_is_number(DECL_INFO.m_data.m_var_simple.m_type_id)) {
					err_puts(("Arithmetic is no number [NOIMPL]"));
					return EMIT_EXPR_FAILURE;
				}

				*wr_type_fetched = DECL_INFO.m_data.m_var_simple.m_type_id;

				if(DECL_INFO.m_data.m_var_simple.m_mask_literal & UTIL_LITERAL_LITERAL) {
					*rwr_literal_opt = DECL_INFO.m_data.m_var_simple.m_literal;
					*wr_newid = DECL_INFO.m_data.m_var_simple.m_init_val;
					return EMIT_EXPR_SUCCESS_LITERAL;
				}

				if(DECL_INFO.m_data.m_var_simple.m_is_predictable) {
					dbg_puts(("Variable is predictable"));
					*wr_newid = DECL_INFO.m_data.m_var_simple.m_init_val;
					return DECL_INFO.m_data.m_var_simple.m_mask_literal & UTIL_LITERAL_CONSTANT
						? EMIT_EXPR_SUCCESS_CONSTANT
						: EMIT_EXPR_SUCCESS;
				}

				dbg_puts(("Load from memory"));
				*wr_newid = h_ctx->m_id++;
				ae2f_expected_but_else(h_ctx->m_count.m_fnimpl = util_emitx_4(
							&h_ctx->m_section.m_fnimpl
							, h_ctx->m_count.m_fnimpl
							, SpvOpLoad
							, DECL_INFO.m_data.m_var_simple.m_type_id
							, *wr_newid
							, DECL_INFO.m_data.m_var_simple.m_id
							)) return EMIT_EXPR_FAILURE;
				return EMIT_EXPR_SUCCESS;
#undef	DECL_INFO
			}
	}

	assert(0 && "no way you came here");
	ae2f_unreachable();

}

#endif
