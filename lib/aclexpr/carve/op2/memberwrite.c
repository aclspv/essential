#include "./memberwrite.h"

#include <aclexpr/carve/util.h>

#include <aclty.h>
#include <aclcur/common.h>
#include <aclcur/load.h>
#include <aclcur/op.h>
#include <aclexpr/curreg.h>

/** capture the first child of the member reference: the base expression
 * (`s` in `s.b`; the base of `s.b.c` is itself a member reference). */
typedef struct {
	CXCursor	m_c_base;
	aclspv_bool_t	m_has;
} s_memberwrite_base_t;

static enum CXChildVisitResult s_memberwrite_base(
		CXCursor			c_cur,
		ae2f_unused CXCursor		c_par,
		CXClientData			rdwr_data
		) {
	s_memberwrite_base_t* ae2f_restrict h = rdwr_data;

	h->m_c_base = c_cur;
	h->m_has = 1;
	return CXChildVisit_Break;
}

/** FieldDecl ordinal of m_c_target inside its record: the member index in
 * the aclty member chain (same walk the memberread visit uses). */
typedef struct {
	CXCursor	m_c_target;
	aclspv_wrd_t	m_idx;
	aclspv_bool_t	m_found;
} s_memberwrite_cnt_t;

static enum CXChildVisitResult s_memberwrite_cnt(
		CXCursor			c_cur,
		ae2f_unused CXCursor		c_par,
		CXClientData			rdwr_data
		) {
	s_memberwrite_cnt_t* ae2f_restrict h = rdwr_data;

	if(c_cur.kind != CXCursor_FieldDecl)
		return CXChildVisit_Continue;

	if(clang_equalCursors(c_cur, h->m_c_target)) {
		h->m_found = 1;
		return CXChildVisit_Break;
	}

	(void)++h->m_idx;
	return CXChildVisit_Continue;
}

aclexpr_carve_lbl_t aclexpr_carve2_memberwrite(
		struct aclexpr_visit_data* const ae2f_restrict	rdwr_data,
		aclspv_wrd_t* const ae2f_restrict	rdwr_prv_idval,
		aclexpr_opbincfg_extra* const ae2f_restrict rdwr_prv_extra,
		ae2f_unused h_scale_t			h_lst_scale,
		aclspv_wrd_t* const ae2f_restrict	rdwr_lst_scale_buf,
		aclexpr_opbincfg_extra* ae2f_restrict rdwr_lst_extra,
		aclutil_literal* ae2f_restrict		r_ltrl,
		aclexpr_carve_lbl_t			c_lbl_continue,
		aclexpr_carve_lbl_t			c_lbl_success,
		aclexpr_carve_lbl_t			c_lbl_err
		) {
	{
		const CXCursor MREF = rdwr_lst_extra[0].m_cursor;
		const CXCursor FD = clang_getCursorReferenced(MREF);
		s_memberwrite_base_t B;
		s_memberwrite_cnt_t CNT;

		B.m_c_base = clang_getNullCursor();
		B.m_has = 0;

		CNT.m_c_target = FD;
		CNT.m_found = 0;
		CNT.m_idx = 0;

		unless(rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_OPCODE] == (aclspv_wrd_t)ACLEXPR_OP2_MEMBERWRITE)
			return c_lbl_continue;

		unless(clang_getCursorKind(MREF) == CXCursor_MemberRefExpr) {
			err_puts("[carve2::memberwrite] lvalue is not a member reference\n");
			return c_lbl_err;
		}

		unless(clang_getCursorKind(FD) == CXCursor_FieldDecl) {
			err_puts("[carve2::memberwrite] member reference is not a field\n");
			return c_lbl_err;
		}

		/** the base must be a plain variable: nested member or array
		 * member writes are deferred (they would need an insert chain). */
		clang_visitChildren(MREF, s_memberwrite_base, &B);

		unless(B.m_has && clang_getCursorKind(B.m_c_base) == CXCursor_DeclRefExpr) {
			err_puts("[carve2::memberwrite] unsupported base expression (nested member / array member write deferred)\n");
			return c_lbl_err;
		}

		{
			const aclspv_id_t REG = aclexpr_curreg_find(
					clang_getCursorReferenced(B.m_c_base)
					, rdwr_data->m_ctx
					);
			h_aclcur_t p_reg = aclcur_get_node(REG, rdwr_data->m_ctx);
			const aclspv_id_t X = rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_END + 1];
			const aclspv_id_t S_VAL = aclcur_load_reg(REG, rdwr_data->m_ctx, 0);

			unless(REG) {
				err_puts("[carve2::memberwrite] no register for the base variable\n");
				return c_lbl_err;
			}

			ae2f_ifezerr(p_reg) {
				err_puts("[carve2::memberwrite] base register missing as node\n");
				return c_lbl_err;
			}

			ae2f_ifnezerr_strict(!X || !S_VAL) {
				err_puts("[carve2::memberwrite] rhs or base value missing\n");
				return c_lbl_err;
			}

			{
				const aclty_t* const ae2f_restrict P_TY
					= aclty_find(p_reg->m_ty, rdwr_data->m_ctx);

				unless(P_TY && P_TY->m_class == ACLTY_STRUCT) {
					err_puts("[carve2::memberwrite] base register is not a composite value\n");
					return c_lbl_err;
				}

				/** member ordinal: FieldDecl children of the record, in
				 * order, matching the aclty member chain — bounded by
				 * the aclty member count. */
				clang_visitChildren(
						clang_getCursorSemanticParent(FD)
						, s_memberwrite_cnt
						, &CNT
						);

				unless(CNT.m_found && CNT.m_idx < P_TY->m_info.m_struct.m_num_mbr) {
					err_puts("[carve2::memberwrite] member index out of the base composite\n");
					return c_lbl_err;
				}

				/** OpCompositeInsert: [op|6, ty(struct), id, obj(comp),
				 * m_idx] — OP3-class emission with the index as an
				 * immediate word. */
				{
					const aclspv_id_t NEW = aclcur_cfg_op2i(
							0
							, SpvOpCompositeInsert
							, p_reg->m_ty
							, X
							, S_VAL
							, CNT.m_idx
							, rdwr_data->m_ctx
							);

					ae2f_ifezerr(NEW) {
						err_puts("[carve2::memberwrite] OpCompositeInsert failed\n");
						return c_lbl_err;
					}

					ae2f_ifezerr(*rdwr_prv_idval = aclcur_store_reg(
								REG
								, NEW
								, rdwr_data->m_ctx
								)) {
						err_puts("[carve2::memberwrite] store of the composite failed\n");
						return c_lbl_err;
					}
				}
			}
		}
	}

	*rdwr_prv_extra = rdwr_lst_extra[1];
	rdwr_prv_extra->m_literal = *r_ltrl;

	return c_lbl_success;
}

