#ifndef	aclexpr_carve_lvalue_h
#define	aclexpr_carve_lvalue_h

#include <aclspv.h>

#include <aclexpr.h>
#include <aclexpr/op.h>
#include <aclexpr/curreg.h>

#include <aclcur/common.h>
#include <aclcur/op.h>
#include <aclcur/store.h>

#include <aclexpr/carve/api.h>

#include <util/log.h>
#include <ae2f/pat/dbg.h>

/** store a compound-assignment (`+=`, `&=`, `<<=`, ...) result back to the
 * l-value the operand-0 extra named.
 *
 * the l-value operand of a compound assignment is carved as an ordinary read,
 * and that read records what the store-back needs on the extra it hands up
 * (extra[0] of the operator's scale). three shapes are possible, in the same
 * order the reads set them up:
 *
 *  - a view-rooted l-value (`*p`, `p->m`, `(*p).m`, `a[i]`):
 *    ACLEXPR_OPCFGX_STATE0_LVALUE_VIEW, and m_extra.m_view.m_reg names the
 *    view register the store walks (aclcur_store_reg).
 *  - a vector element (`v[i]`): ACLEXPR_OPCFGX_STATE0_LVALUE_VEC_ELM. a
 *    vector is a value with no address, so there is nothing to store
 *    *through*: the whole vector is rebuilt (OpVectorInsertDynamic of VAL at
 *    the read's index into the read's vector value) and stored back to the
 *    register.
 *  - a plain variable: its register is re-derived from the assignment cursor,
 *    exactly as it was before the view/vector forms existed.
 *
 * @param	rdwr_lst_extra	the operator's operand-0 extra (the l-value read)
 * @param	VAL		the computed new value of the l-value
 * @param	h_ctx		the compile context
 * @param	c_tag		a caller-supplied tag for the failure log
 *
 * @return	non-zero when the store landed, 0 otherwise (m_err is set).
 * */
ae2f_inline_f static aclspv_bool_t aclexpr_lvalue_storeback(
		aclexpr_opbincfg_extra* ae2f_restrict	rdwr_lst_extra,
		const aclspv_id_t			VAL,
		const h_aclspv_ctx_t			h_ctx,
		const char*				c_tag
		) {
	/** view-rooted l-value: the read built the view naming the region, so
	 * the store goes through it, exactly like a plain variable's. */
	if(rdwr_lst_extra->m_state0
			& (aclspv_wrd_t)ACLEXPR_OPCFGX_STATE0_LVALUE_VIEW) {
		unless(aclcur_store_reg(
					rdwr_lst_extra->m_extra.m_view.m_reg
					, VAL
					, h_ctx)) {
			err_puts(c_tag);
			h_ctx->m_err = ACLSPV_COMPILE_UNHANDLED_CARVE;
			return 0;
		}

		return 1;
	}

	/** vector element: rebuild the whole vector around the new element and
	 * store that. the vector value is the one the read already resolved —
	 * NOT re-resolved here, both because the register cannot have changed in
	 * between and because resoling a view address is emit-time and
	 * non-idempotent. */
	if(rdwr_lst_extra->m_state0
			& (aclspv_wrd_t)ACLEXPR_OPCFGX_STATE0_LVALUE_VEC_ELM) {
		aclspv_id_t NEW_VEC;

		/** OpVectorInsertDynamic: result type = the vector type, operands
		 * = (vector, component value, index) — all three are value IDS, so
		 * op3, not the immediate form. */
		unless(NEW_VEC = aclcur_cfg_op3(
					0
					, SpvOpVectorInsertDynamic
					, rdwr_lst_extra->m_extra.m_vecelm.m_ty
					, rdwr_lst_extra->m_extra.m_vecelm.m_vec
					, VAL
					, rdwr_lst_extra->m_extra.m_vecelm.m_idx
					, h_ctx)) {
			err_puts(c_tag);
			h_ctx->m_err = ACLSPV_COMPILE_UNHANDLED_CARVE;
			return 0;
		}

		unless(aclcur_store_reg(
					rdwr_lst_extra->m_extra.m_vecelm.m_reg
					, NEW_VEC
					, h_ctx)) {
			err_puts(c_tag);
			h_ctx->m_err = ACLSPV_COMPILE_UNHANDLED_CARVE;
			return 0;
		}

		return 1;
	}

	/** plain variable: the l-value cursor names the variable. */
	{

	const aclspv_id_t REG = aclexpr_curreg_find(
			clang_getCursorReferenced(rdwr_lst_extra->m_cursor)
			, h_ctx
			);

	unless(REG) {
		err_puts(c_tag);
		h_ctx->m_err = ACLSPV_COMPILE_UNHANDLED_CARVE;
		return 0;
	}

	unless(aclcur_store_reg(REG, VAL, h_ctx)) {
		err_puts(c_tag);
		h_ctx->m_err = ACLSPV_COMPILE_UNHANDLED_CARVE;
		return 0;
	}

	}

	return 1;
}

#endif
