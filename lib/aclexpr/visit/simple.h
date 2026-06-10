/** 
 * @file	simple1.h
 * @brief	int & float literals, variable references
 * */

#ifndef aclexpr_visit_simple1_h
#define aclexpr_visit_simple1_h

#include <util/emitx.h>
#include <util/constant.h>

#include <aclexpr.h>
#include <aclexpr/op.h>

#include <aclcursor.h>

#include <.aclspv/util/mkname.h>
#include <aclexpr/visit/api.h>

/** 
 * binary expression with 1 operators.
 * */
ae2f_inline_f static aclexpr_visit_lbl_t aclexpr_visit_op0_val(
		const CXCursor			c_cur,
		const h_aclspv_ctx_t		h_ctx,
		aclexpr_opbincfg_extra*		rdwr_extra,
#define		wr_type_fetched			(&(rdwr_extra)->m_type_original)
#define		rwr_literal			(&(rdwr_extra)->m_literal)
#define		r_literal_mask			(&(rdwr_extra)->m_mask_literal)

		aclspv_id_t* ae2f_restrict const	wr_newid,
		const aclexpr_visit_lbl_t		c_lbl_success,
		const aclexpr_visit_lbl_t		c_lbl_fail,
		const aclexpr_visit_lbl_t		c_lbl_pass
		) {
	aclexpr_literal EVRES;
	ae2fpat_assertassume(wr_newid);
	ae2fpat_assertassume(h_ctx);
	ae2fpat_assertassume(rdwr_extra);

	rdwr_extra->m_mask_literal = ACLUTIL_LITERAL_MASK_LITERAL;

	switch((uint_leastenum_t)c_cur.kind) {
		/** unknown */
		default:
			return c_lbl_pass;

			/** literals */

		case CXCursor_IntegerLiteral:
		case CXCursor_FloatingLiteral:
			{
				CXEvalResult EVAL = clang_Cursor_Evaluate(c_cur);

				/** no cleanup needed. init already fucked. */
				ae2f_expected_but_else (EVAL) {
					err_puts("[visit::0::val] clang_Cursor_Evaluate failed.");
					return c_lbl_fail;
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
								? ACLSPV_ID_I32 : ACLSPV_ID_I64);

						if(rwr_literal) *rwr_literal = EVRES;

						ae2fpat_assertassume_strict(*wr_type_fetched);

						ae2f_expected_but_else(aclid_mk_default_id(
									*wr_type_fetched
									, h_ctx)) {
							return c_lbl_fail;
						}

						switch(aclid_get_bit_width(*wr_type_fetched)) {
							case 32:
								dbg_puts("I32");
								ae2f_expected_but_else(*wr_newid = aclutil_mk_constant_val_id(
											(aclspv_wrd_t)EVRES.m_u64
											, h_ctx
											))
									return c_lbl_fail;

								return c_lbl_success;

							case 64:
								dbg_puts("I64");
								ae2f_expected_but_else(*wr_newid = aclutil_mk_constant_val64_id(
											(aclspv_wrd_t)EVRES.m_u64
											, h_ctx
											))
									return c_lbl_fail;

								return c_lbl_success;
							default:
								return c_lbl_fail;
						}

					case CXEval_Float:
						dbg_puts(("Literal: double (float casted)"));
						dbg_prefix(a);

						if(aclspv_chck_capability_from_ctx(h_ctx, SpvCapabilityFloat64)) {
							EVRES.m_flt = (float)clang_EvalResult_getAsDouble(EVAL);
							*wr_type_fetched = ACLSPV_ID_F32;
							dbg_call(fprintf, (stderr, "Literal fetched: %f\n", (float)EVRES.m_flt));
						} else {
							EVRES.m_dbl = clang_EvalResult_getAsDouble(EVAL);
							*wr_type_fetched = ACLSPV_ID_F64;
							dbg_call(fprintf, (stderr, "Literal fetched: %f\n", (float)EVRES.m_dbl));
						}

						clang_EvalResult_dispose(EVAL);

						ae2fpat_assertassume_strict(*wr_type_fetched);

						*rwr_literal = EVRES;

						ae2f_expected_but_else(*wr_newid = aclutil_mk_constant_val_f32_id(
									EVRES.m_flt
									, h_ctx
									))
							return c_lbl_fail;

						return c_lbl_success;

					default:
						return c_lbl_fail;
				}
			} assert(0); ae2f_unreachable();

		case CXCursor_DeclRefExpr:
			if(rdwr_extra->m_state0 & ACLEXPR_OPCFGX_STATE0_VAL_NO_NEED) {
				dbg_puts("[visit0::val value not needed]");
				return c_lbl_success;
			} else {
				const CXCursor DECL = clang_getCursorReferenced(c_cur);
				const aclspv_wrd_t	DECL_IDX = aclutil_find_cursor(
						h_ctx->m_num_cursor
						, h_ctx->m_cursors.m_p
						, DECL
						);

				ae2f_expected_but_else(DECL_IDX < h_ctx->m_num_cursor)
					return c_lbl_fail;

#define	DECL_INFO	DECL_IDX[((aclutil_cursor* ae2f_restrict)h_ctx->m_cursors.m_p)]

				/** NaN is undefined behaviour here */
				unless(aclid_is_arithmetic(DECL_INFO.m_data.m_vardecl.m_type_id, h_ctx)) {
					err_prefix(a);
					err_call((void), (fprintf(stderr
									, "[visit0::val] type(%u) is no arithmetic [NOIMPL]\n"
									, DECL_INFO.m_data.m_vardecl.m_type_id))
						);
					return c_lbl_fail;
				}

				*wr_type_fetched = DECL_INFO.m_data.m_vardecl.m_type_id;
				*rwr_literal = DECL_INFO.m_data.m_vardecl.m_literal;

				dbg_prefix();
				dbg_call(, fprintf(stderr, "[visit0::val] lval cache check %u value %u idx %u before\n"
							, (unsigned)!!(DECL_INFO.m_data.m_vardecl.m_mask 
								& (aclspv_wrd_t)ACLUTIL_CURSOR_VAR_PREDICTABLE)
							, (unsigned)*wr_newid
							, (unsigned)DECL_IDX));

				ae2f_ifezerr(*wr_newid = aclutil_assign_cursor_lval(
							&DECL_INFO, h_ctx
							, 0, 0
							, 3, 0
							, rwr_literal))
					return c_lbl_fail;

				dbg_prefix();
				dbg_call(, fprintf(stderr, "[visit0::val] lval cache check %u value %u idx %u\n"
							, (unsigned)!!(DECL_INFO.m_data.m_vardecl.m_mask 
								& (aclspv_wrd_t)ACLUTIL_CURSOR_VAR_PREDICTABLE)
							, (unsigned)*wr_newid
							, (unsigned)DECL_IDX));

				rdwr_extra->m_mask_literal = ((aclspv_wrd_t)(ACLUTIL_CURSOR_VAR_LITERAL) & DECL_INFO.m_data.m_vardecl.m_mask);
				return c_lbl_success;
#undef	DECL_INFO
			}
	}

	assert(0 && "no way you came here");
	ae2f_unreachable();

}

#endif
