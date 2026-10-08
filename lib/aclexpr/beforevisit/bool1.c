#include "./bool1.h"
#include "./util.h"

#include <aclcur/common.h>
#include <aclcur/reg/var.h>

#include <aclexpr/op.h>
#include <.aclspv/util/mkname.h>

#include <ae2f/pat/dbg.h>

#include <util/scale.h>
#include <util/constant.h>

aclspv_bool_t aclexpr_beforevisit2_bool1(
		struct aclexpr_visit_data* const ae2f_restrict	rdwr_data,
		aclspv_wrd_t					c_opidx
		) {
#if	1
	h_scale_t LST_SCALE				= get_last_scale_from_vec(&CMDSTCK_SCALE(rdwr_data->m_ctx));
	aclspv_wrd_t* ae2f_restrict LST_SCALE_BUF	= get_buf_from_scale2(
			aclspv_wrd_t
			, &CMDSTCK_SCALE(CTX(rdwr_data))
			, *LST_SCALE
			);

	aclexpr_opbincfg_extra* ae2f_restrict LST_EXTRA = (void* ae2f_restrict)(LST_SCALE_BUF + 2 + ACLEXPR_OPBINCFG_END);

	aclspv_id_t	NEWID_0 = 0;
	struct aclutil_cxtp_veccfg_t V;
	aclspv_id_t VBOOL;

	aclspv_bool_t	IS_LWOR = LST_SCALE_BUF[ACLEXPR_OPBINCFG_OPCODE] == ACLEXPR_OP2_OR_L;
	aclspv_id_t	LWBR[2];

	dbg_prefix(a);
	dbg_call(fprintf, (stderr, "opidx: %u\n", (unsigned)c_opidx));

	unless(c_opidx)		return 0;

	V = aclty_get_veccfg(
			LST_EXTRA[0].m_type_original
			, rdwr_data->m_ctx
			);

	VBOOL = aclty_mk_vec(ACLSPV_ID_BOOL, V.m_num, rdwr_data->m_ctx);

	if(1 != V.m_num)	return 0;	/** f*ck short circuting when it is vector */

	switch(aclexpr_arithmetic_cast(
			LST_SCALE_BUF[ACLEXPR_OPBINCFG_END]
			, LST_EXTRA[0].m_type_original
			, LST_EXTRA[0].m_mask_literal
			, LST_EXTRA[0].m_literal
			, &LST_SCALE_BUF[ACLEXPR_OPBINCFG_END]
			, VBOOL
			, &LST_EXTRA[0].m_literal
			, rdwr_data->m_ctx
			)) {
		case ACLEXPR_RET_SUCCESS_LITERAL:
			fixme_puts("make literal version");
			ae2f_fallthrough;
		case ACLEXPR_RET_SUCCESS_CONSTANT:
			LST_EXTRA[0].m_mask_literal = ACLEXPR_RET_SUCCESS_CONSTANT;
			break;
		case ACLEXPR_RET_SUCCESS:
			LST_EXTRA[0].m_mask_literal &= ACLEXPR_RET_SUCCESS;
			break;

		case ACLEXPR_RET_FAILURE:
		case ACLEXPR_RET_NOT_THE_CASE:
			err_puts("[beforevisit::bool1] failed casting");
			return 1;

		default:
			ae2fpat_assertassume_strict(0 && "[beforevisit::bool1] arithmetic cast");
	}

	NEWID_0 =	LST_SCALE_BUF[ACLEXPR_OPBINCFG_END];

	ae2fpat_assertassume_strict(LST_EXTRA[0].m_type_original);
	ae2fpat_assertassume_strict(NEWID_0);

	/** return value was not literal */
	LST_EXTRA[0].m_type_original = VBOOL;
	LWBR[0] = rdwr_data->m_ctx->m_id++;
	LWBR[1] = rdwr_data->m_ctx->m_id++;

	_aclutil_mkname_dbg(
			L0, LWBR[0]
			, "lwblk1", *rdwr_data->m_ctx
			, rdwr_data->m_ctx->m_tmp.m_v0
			);

	_aclutil_mkname_dbg(
			L1, LWBR[1]
			, "lwblk0", *rdwr_data->m_ctx
			, rdwr_data->m_ctx->m_tmp.m_v0
			);

	fixme_puts("add control variable later");

	{
		/** value-phi: a temporary register carries the boolean result. */
		const aclcur_reg_t REGIN = aclcur_regvar_in();

		ae2f_ifezerr(LST_SCALE_BUF[ACLEXPR_OPBINCFG_RESID] = aclcur_cfg_reg(
				clang_getNullCursor()
				, ACLSPV_ID_BOOL
				, SpvStorageClassFunction
				, &REGIN
				, rdwr_data->m_ctx
				)) return 1;
	}

	/** 0 must be fallback value. */
	ae2f_ifezerr(LST_SCALE_BUF[ACLEXPR_OPBINCFG_END] = aclval_cfg_bool(IS_LWOR, rdwr_data->m_ctx))
		return 1;

	/** the fallback is stored on the branch-ALL block, before the branch. */
	ae2f_ifezerr(aclcur_store_reg(
			LST_SCALE_BUF[ACLEXPR_OPBINCFG_RESID]
			, LST_SCALE_BUF[ACLEXPR_OPBINCFG_END]
			, rdwr_data->m_ctx
			)) return 1;

	/** branch-ALL: from where the short-circuit falls. */
	LST_EXTRA[0].m_extra.m_bool1.m_parent = aclcur_get_lbl_last(rdwr_data->m_ctx);

	/** terminate branch-ALL with the (possibly constant) condition. */
	ae2f_ifezerr(aclcur_cfg_jc(
			NEWID_0
			, LWBR[IS_LWOR]
			, LWBR[!IS_LWOR]
			, 0, 0
			, LWBR[1], 0, 0
			, rdwr_data->m_ctx
			)) return 1;

	/** branch-CUR starts here. */
	ae2f_ifezerr(aclcur_cfg_lbl(LWBR[0], rdwr_data->m_ctx))
		return 1;

	LST_EXTRA[1].m_extra.m_bool1.m_parent	= LWBR[0];			/** branch-current */
	rdwr_data->m_fnctx->m_branch		= LWBR[1];

	/** no sync-caching on vardecl is required since there's no other branch */
	return 0;
#else
	(void)rdwr_data;
	(void)c_opidx;
	return 0;	/** stub */
#endif
}
