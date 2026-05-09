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
#include <.aclspv/util/mkname.h>

/** 
 * binary expression with 1 operators.
 * */
ae2f_inline_f static enum EMIT_EXPR_ s_simple(
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
									, (unsigned long)EVRES.m_api_uintmax
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
						dbg_prefix(a);

						if(aclspv_chck_capability_from_ctx(h_ctx, SpvCapabilityFloat64)) {
							EVRES.m_flt = (float)clang_EvalResult_getAsDouble(EVAL);
							*wr_type_fetched = ID_DEFAULT_F32;
							dbg_call(fprintf, (stderr, "Literal fetched: %f\n", (float)EVRES.m_flt));
						} else {
							EVRES.m_dbl = clang_EvalResult_getAsDouble(EVAL);
							*wr_type_fetched = ID_DEFAULT_F64;
							dbg_call(fprintf, (stderr, "Literal fetched: %f\n", (float)EVRES.m_dbl));
						}

						clang_EvalResult_dispose(EVAL);

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
				unless(aclid_is_arithmetic(DECL_INFO.m_data.m_vardecl.m_type_id, h_ctx)) {
					err_prefix(a);
					err_call((void), (fprintf(stderr
									, "Type(%u) is no arithmetic [NOIMPL]\n"
									, DECL_INFO.m_data.m_vardecl.m_type_id))
						);
					return EMIT_EXPR_FAILURE;
				}

				*wr_type_fetched = DECL_INFO.m_data.m_vardecl.m_type_id;
				ae2f_expected_if(*wr_newid = aclutil_assign_cursor_lval(
							&DECL_INFO, h_ctx
							, 0, 0
							, 0, 0
							, rwr_literal_opt)) 
					switch(DECL_INFO.m_data.m_vardecl.m_mask & ACLUTIL_CURSOR_VAR_LITERAL) {
						case UTIL_LITERAL_MASK_LITERAL:
						case UTIL_LITERAL_LITERAL:

							return EMIT_EXPR_SUCCESS_LITERAL;

						case UTIL_LITERAL_CONSTANT:
							return EMIT_EXPR_SUCCESS_CONSTANT;

						default:
							return EMIT_EXPR_SUCCESS;
					}


				return EMIT_EXPR_FAILURE;
#undef	DECL_INFO
			}
	}

	assert(0 && "no way you came here");
	ae2f_unreachable();

}

#endif
