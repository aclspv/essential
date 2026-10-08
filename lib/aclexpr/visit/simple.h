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

#include <aclcur/common.h>
#include <aclcur/load/get.h>
#include <aclexpr/curreg.h>

#include <.aclspv/util/mkname.h>
#include <aclexpr/visit/api.h>

/** 
 * binary expression with 1 operators.
 * */
ae2f_inline_f static aclexpr_visit_lbl_t aclexpr_visit_op0_val(
		const CXCursor			c_cur,
		const aclutil_fnctx* ae2f_restrict	h_fnctx,
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
	ae2fpat_assertassume(h_fnctx);
	ae2fpat_assertassume(h_fnctx->m_ctx);
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
									, h_fnctx->m_ctx)) {
							return c_lbl_fail;
						}

						switch(aclid_get_bit_width(*wr_type_fetched)) {
							case 32:
								dbg_puts("I32");
								ae2f_expected_but_else(*wr_newid = aclutil_mk_constant_val_id(
											(aclspv_wrd_t)EVRES.m_u64
											, h_fnctx->m_ctx
											))
									return c_lbl_fail;

								return c_lbl_success;

							case 64:
								dbg_puts("I64");
								ae2f_expected_but_else(*wr_newid = aclutil_mk_constant_val64_id(
											(aclspv_wrd_t)EVRES.m_u64
											, h_fnctx->m_ctx
											))
									return c_lbl_fail;

								return c_lbl_success;
							default:
								return c_lbl_fail;
						}

					case CXEval_Float:
						dbg_puts(("Literal: double (float casted)"));
						dbg_prefix(a);

						if(aclspv_chck_capability_from_ctx(h_fnctx->m_ctx, SpvCapabilityFloat64)) {
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

						ae2f_expected_but_else(*wr_newid = aclval_cfg_f32(
									EVRES.m_flt
									, h_fnctx->m_ctx
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
				/** no value fetched: not a compile-time literal either. */
				rdwr_extra->m_mask_literal = 0;
				return c_lbl_success;
			} else {
				const CXCursor DECL = clang_getCursorReferenced(c_cur);
				const aclspv_id_t REG = aclexpr_curreg_find(DECL, h_fnctx->m_ctx);

				ae2f_expected_but_else(REG)
					return c_lbl_fail;

				{
					h_aclcur_t p_reg = aclcur_get_node(REG, h_fnctx->m_ctx);

					ae2f_ifezerr(p_reg)	return c_lbl_fail;

					*wr_type_fetched = p_reg->m_ty;
				}

				/** loaded from memory: runtime value — unless the served
				 * id is a compile-time constant: a store of a literal
				 * tracks the OpConstant itself, so a reference to the
				 * register inherits the constant's literalness. the
				 * register's own stale label/loop state (phis, CHANGED)
				 * must not force a runtime op. */
				ae2f_ifezerr(*wr_newid = aclcur_load_reg(
						REG, h_fnctx->m_ctx, rwr_literal))
					return c_lbl_fail;

				{
					const aclcur_t LB = aclcur_get_load(*wr_newid, h_fnctx->m_ctx);

					if((LB.m_val_mask & ACLUTIL_LITERAL_MASK_LITERAL)
							== ACLUTIL_LITERAL_MASK_LITERAL) {
						rdwr_extra->m_mask_literal = ACLUTIL_LITERAL_MASK_LITERAL;
						if(rwr_literal)
							*rwr_literal = LB.m_val_lit;
					} else if(LB.m_val_mask & ACLUTIL_LITERAL_CONSTANT) {
						rdwr_extra->m_mask_literal = ACLUTIL_LITERAL_MASK_CONSTANT;
					} else {
						rdwr_extra->m_mask_literal = 0;
					}
				}
				return c_lbl_success;
			}
	}

	assert(0 && "no way you came here");
	ae2f_unreachable();

}

#endif
