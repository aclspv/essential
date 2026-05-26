/** @file expr.c */

#include "./expr.h"

#include <ae2f/bll.h>

#include <util/op.h>
#include <util/scale.h>
#include <util/emitx.h>
#include <util/log.h>
#include <util/iddef.h>
#include <util/visitfromroot.h>

enum EMIT_EXPR_ aclspv_get_expr2(
		aclspv_id_t* ae2f_restrict const	ret_id
		, aclspv_id_t* ae2f_restrict const	r_type_opt
		, const CXCursor			c_cur
		, util_literal* ae2f_restrict		wr_literal_opt
		, h_aclspv_ctx_t			h_ctx
		) {
	aclspv_wrd_t* ae2f_restrict SCALE_BOOT;
	struct aclexpr_visit_data	DATA;

	ae2f_ifezerr(h_ctx)	return EMIT_EXPR_FAILURE;

	DATA.m_ctx		= h_ctx;			/* context. must be pointer to context. */
	DATA.m_exprcount	= 0;				/* stack count. must be 0. */
	DATA.m_id_ret		= 0;				/* result value id */
	DATA.m_literal_flag	= UTIL_LITERAL_MASK_LITERAL;	/* see enum UTIL_LITERAL_MASK_ */
	DATA.m_tyid_current	= 0;				/* type casted */
	DATA.m_num_cmdcounter	= 1;				/* count for operand */

	ae2f_expected_but_else(h_ctx->m_tmp.m_v3.m_p)
		return EMIT_EXPR_FAILURE;

	ae2f_expected_but_else(h_ctx->m_tmp.m_v1.m_p)
		return EMIT_EXPR_FAILURE;

	ae2f_expected_but_else(init_scale(&h_ctx->m_tmp.m_v1
				, count_to_sz(UTIL_OPBINCFG_END) + sizeof(aclspv_wrd_t)
				+ sizeof(util_opbincfg_extra)
				))
		return EMIT_EXPR_FAILURE;

	SCALE_BOOT = get_buf_from_scale2(aclspv_wrd_t, &h_ctx->m_tmp.m_v1, *get_last_scale_from_vec(&h_ctx->m_tmp.m_v1));
	ae2fpat_assertassume(SCALE_BOOT);
	ae2fpat_assertassume(!get_last_scale_from_vec(&h_ctx->m_tmp.m_v1)->m_id);

	SCALE_BOOT[UTIL_OPBINCFG_NUMOPRND]		= 1;
	SCALE_BOOT[UTIL_OPBINCFG_MASKLITERAL]		= UTIL_LITERAL_MASK_LITERAL;
	SCALE_BOOT[UTIL_OPBINCFG_RESTYID]		= 0;
	SCALE_BOOT[UTIL_OPBINCFG_RESID]			= 0;
	SCALE_BOOT[UTIL_OPBINCFG_OPCODE]		= 0;
#define INIT_EXTRA ((util_opbincfg_extra* ae2f_restrict)(void* ae2f_restrict)(&SCALE_BOOT[UTIL_OPBINCFG_END + 1]))

	INIT_EXTRA->m_mask_literal		= 3;
	INIT_EXTRA->m_type_original		= 0;
	INIT_EXTRA->m_literal.m_api_uintmax	= 0;

	((umax* ae2f_restrict)h_ctx->m_tmp.m_v2.m_p)[0] = 0;

	dbg_prefix(a);
	dbg_call(fprintf, (stderr
				, "Initialising first constant sector %u[%p]\n"
				, SCALE_BOOT[1], (void*)(SCALE_BOOT + 1))
		);

	(void)aclutil_visitfromroot(c_cur, aclexpr_visit, &DATA);
	ae2f_ifnezerr(h_ctx->m_err) return EMIT_EXPR_FAILURE;

	if(wr_literal_opt)
		*wr_literal_opt = INIT_EXTRA->m_literal;

	if(r_type_opt)
		*r_type_opt = DATA.m_tyid_current;

	ae2fpat_assertassume_strict(ret_id);
	*ret_id = DATA.m_id_ret;

	return		DATA.m_literal_flag & UTIL_LITERAL_LITERAL ? EMIT_EXPR_SUCCESS_LITERAL
		: 	DATA.m_literal_flag & UTIL_LITERAL_CONSTANT ? EMIT_EXPR_SUCCESS_CONSTANT
		: 	EMIT_EXPR_SUCCESS;

#undef	INIT_EXTRA
}

enum EMIT_EXPR_ aclspv_get_expr_btcast(
		aclspv_id_t* ae2f_restrict const ret_id
		, const aclspv_id_t c_type_req
		, const CXCursor c_cur
		, util_literal* ae2f_restrict	wr_literal_opt
		, h_util_ctx_t h_ctx
		) 
{
	aclspv_wrd_t* ae2f_restrict SCALE_BOOT;
	struct aclexpr_visit_data	DATA;

	ae2f_ifezerr(h_ctx)	return EMIT_EXPR_FAILURE;

	DATA.m_ctx		= h_ctx;			/* context. must be pointer to context. */
	DATA.m_exprcount	= 0;				/* stack count. must be 0. */
	DATA.m_id_ret		= 0;				/* result value id */
	DATA.m_literal_flag	= UTIL_LITERAL_MASK_LITERAL;	/* see enum UTIL_LITERAL_MASK_ */
	DATA.m_tyid_current	= 0;				/* type casted */
	DATA.m_num_cmdcounter	= 1;				/* count for operand */

	ae2f_expected_but_else(h_ctx->m_tmp.m_v3.m_p)
		return EMIT_EXPR_FAILURE;

	ae2f_expected_but_else(h_ctx->m_tmp.m_v1.m_p)
		return EMIT_EXPR_FAILURE;

	ae2f_expected_but_else(init_scale(&h_ctx->m_tmp.m_v1
				, count_to_sz(UTIL_OPBINCFG_END) + sizeof(aclspv_wrd_t)
				+ sizeof(util_opbincfg_extra)
				))
		return EMIT_EXPR_FAILURE;

	SCALE_BOOT = get_buf_from_scale2(aclspv_wrd_t, &h_ctx->m_tmp.m_v1, *get_last_scale_from_vec(&h_ctx->m_tmp.m_v1));
	ae2fpat_assertassume_strict(!!SCALE_BOOT);
	ae2fpat_assertassume_strict(!get_last_scale_from_vec(&h_ctx->m_tmp.m_v1)->m_id);

	SCALE_BOOT[UTIL_OPBINCFG_NUMOPRND]		= 1;
	SCALE_BOOT[UTIL_OPBINCFG_MASKLITERAL]		= UTIL_LITERAL_MASK_LITERAL;
	SCALE_BOOT[UTIL_OPBINCFG_RESTYID]		= 0;
	SCALE_BOOT[UTIL_OPBINCFG_RESID]			= 0;
	SCALE_BOOT[UTIL_OPBINCFG_OPCODE]		= 0;
#define INIT_EXTRA ((util_opbincfg_extra* ae2f_restrict)(void* ae2f_restrict)(&SCALE_BOOT[UTIL_OPBINCFG_END + 1]))

	INIT_EXTRA->m_mask_literal		= 3;
	INIT_EXTRA->m_type_original		= 0;
	INIT_EXTRA->m_literal.m_api_uintmax	= 0;

#undef	INIT_EXTRA

	((umax* ae2f_restrict)h_ctx->m_tmp.m_v2.m_p)[0] = 0;

	dbg_prefix(a);
	dbg_call(fprintf
			, (stderr
				, "Initialising first constant sector %u[%p]\n"
				, SCALE_BOOT[1]
				, (void*)(SCALE_BOOT + 1))
		);

	(void)aclutil_visitfromroot(c_cur, aclexpr_visit, &DATA);
	ae2f_ifnezerr(h_ctx->m_err) return EMIT_EXPR_FAILURE;

	/** bitcast is not required when two types are exactly same */
	unless(DATA.m_tyid_current != c_type_req)
		return	aclexpr_arithmetic_cast(
				DATA.m_id_ret
				, DATA.m_tyid_current
				, DATA.m_id_ret
				, DATA.m_ret_literal 
				, ret_id
				, c_type_req
				, wr_literal_opt
				, h_ctx);

	ae2f_expected_but_else(aclid_is_number(c_type_req) 
			&& aclid_is_number(DATA.m_tyid_current)) {
		return EMIT_EXPR_NOT_THE_CASE;
	}

	if(DATA.m_literal_flag & UTIL_LITERAL_LITERAL) {
		if(wr_literal_opt)
			*wr_literal_opt = DATA.m_ret_literal;

		return aclexpr_arithmetic_cast_literal(
				c_type_req
				, DATA.m_ret_literal
				, ret_id
				, c_type_req
				, wr_literal_opt
				, h_ctx
				);
	}

	/** NON-LITERAL */
	{
		const aclspv_id_t	TYPE_SRC = aclid_getty_unsigned(
				aclid_get_bit_width(DATA.m_tyid_current)
				);

		const aclspv_id_t	TYPE_DST = aclid_getty_unsigned(aclid_get_bit_width(c_type_req));

		const aclspv_id_t	VAL_SRC = h_ctx->m_id;
		typedef const aclspv_id_t* ae2f_restrict ptr_id_t;

		ptr_id_t VAL_SRC_PTR, TYPE_SRC_PTR;
		aclspv_id_t	VAL_DST;

		ae2f_expected_but_else(aclid_mk_default_id(TYPE_DST, h_ctx)
				&& aclid_mk_default_id(TYPE_SRC, h_ctx)) {
			return EMIT_EXPR_FAILURE;
		}

		unless(aclid_is_int(DATA.m_tyid_current)) {
			++h_ctx->m_id;
			ae2f_expected_but_else(h_ctx->m_count.m_fnimpl = util_emitx_4(
						&h_ctx->m_section.m_fnimpl
						, h_ctx->m_count.m_fnimpl
						, SpvOpBitcast
						, TYPE_SRC
						, VAL_SRC
						, DATA.m_id_ret 
						)) {
				return EMIT_EXPR_FAILURE;
			}

			VAL_SRC_PTR = &VAL_SRC;
			TYPE_SRC_PTR = &TYPE_SRC;
		} else {
			VAL_SRC_PTR = &DATA.m_id_ret;
			TYPE_SRC_PTR = &DATA.m_tyid_current;
		}

		ae2fpat_assertassume(aclid_is_int(*TYPE_SRC_PTR));
		switch(aclexpr_arithmetic_cast_non_literal(
					*VAL_SRC_PTR
					, *TYPE_SRC_PTR
					, 0
					, &VAL_DST
					, TYPE_DST
					, h_ctx
					)) {
			default:
			case EMIT_EXPR_NOT_THE_CASE:
				return EMIT_EXPR_NOT_THE_CASE;
			case EMIT_EXPR_FAILURE:
				return EMIT_EXPR_FAILURE;

			case EMIT_EXPR_SUCCESS:
			case EMIT_EXPR_SUCCESS_CONSTANT:
			case EMIT_EXPR_SUCCESS_LITERAL:
				break;
		}

		ae2fpat_assertassume(aclid_is_unsigned(TYPE_DST));

		unless(aclid_is_int(c_type_req)) {
			ae2f_expected_but_else(h_ctx->m_count.m_fnimpl = util_emitx_4(
						&h_ctx->m_section.m_fnimpl
						, h_ctx->m_count.m_fnimpl
						, SpvOpBitcast
						, c_type_req
						, *ret_id = h_ctx->m_id++
						, VAL_DST
						)) {
				return EMIT_EXPR_FAILURE;
			}

			return EMIT_EXPR_SUCCESS;
		} else {
			return aclexpr_arithmetic_cast_non_literal(
					VAL_DST
					, TYPE_DST
					, 0
					, ret_id
					, c_type_req
					, h_ctx
					);
		}
	}

	ae2f_unreachable();
}

enum EMIT_EXPR_ aclspv_get_expr(
		aclspv_id_t* ae2f_restrict const ret_id
		, const aclspv_id_t c_type_req
		, const CXCursor c_cur
		, util_literal* ae2f_restrict	wr_literal_opt
		, h_util_ctx_t h_ctx
		) {
	aclspv_wrd_t* ae2f_restrict SCALE_BOOT;
	struct aclexpr_visit_data	DATA;

	DATA.m_ctx		= h_ctx;			/* context. must be pointer to context. */
	DATA.m_exprcount	= 0;				/* stack count. must be 0. */
	DATA.m_id_ret		= 0;				/* result value id */
	DATA.m_literal_flag	= UTIL_LITERAL_MASK_LITERAL;	/* see enum UTIL_LITERAL_MASK_ */
	DATA.m_tyid_current	= 0;				/* type casted */
	DATA.m_num_cmdcounter	= 1;				/* count for operand */

	ae2f_expected_but_else(h_ctx->m_tmp.m_v3.m_p)
		return EMIT_EXPR_FAILURE;

	ae2f_expected_but_else(h_ctx->m_tmp.m_v1.m_p)
		return EMIT_EXPR_FAILURE;

	ae2f_expected_but_else(init_scale(&h_ctx->m_tmp.m_v1
				, count_to_sz(UTIL_OPBINCFG_END) + sizeof(aclspv_wrd_t)
				+ sizeof(util_opbincfg_extra)
				))
		return EMIT_EXPR_FAILURE;

	SCALE_BOOT = get_buf_from_scale2(aclspv_wrd_t, &h_ctx->m_tmp.m_v1, *get_last_scale_from_vec(&h_ctx->m_tmp.m_v1));
	ae2fpat_assertassume_strict(!!SCALE_BOOT);
	ae2fpat_assertassume_strict(!get_last_scale_from_vec(&h_ctx->m_tmp.m_v1)->m_id);

	SCALE_BOOT[UTIL_OPBINCFG_NUMOPRND]		= 1;
	SCALE_BOOT[UTIL_OPBINCFG_MASKLITERAL]		= UTIL_LITERAL_MASK_LITERAL;
	SCALE_BOOT[UTIL_OPBINCFG_RESTYID]		= 0;
	SCALE_BOOT[UTIL_OPBINCFG_RESID]			= 0;
	SCALE_BOOT[UTIL_OPBINCFG_OPCODE]		= 0;
#define INIT_EXTRA ((util_opbincfg_extra* ae2f_restrict)(void* ae2f_restrict)(&SCALE_BOOT[UTIL_OPBINCFG_END + 1]))

	INIT_EXTRA->m_mask_literal		= 3;
	INIT_EXTRA->m_type_original		= 0;
	INIT_EXTRA->m_literal.m_api_uintmax	= 0;

#undef	INIT_EXTRA

	((umax* ae2f_restrict)h_ctx->m_tmp.m_v2.m_p)[0] = 0;

	dbg_prefix(a);
	dbg_call(fprintf
			, (stderr
				, "Initialising first constant sector %u[%p]\n"
				, SCALE_BOOT[1]
				, (void*)(SCALE_BOOT + 1))
		);

	(void)aclutil_visitfromroot(c_cur, aclexpr_visit, &DATA);
	ae2f_ifnezerr(h_ctx->m_err) return EMIT_EXPR_FAILURE;

	unless(c_type_req)
		return ae2f_bll_sel_bool1(uint_fastenum_t, DATA.m_literal_flag & 2, EMIT_EXPR_SUCCESS_LITERAL, 0u)
		| ae2f_bll_sel_bool1(uint_fastenum_t, DATA.m_literal_flag & 1, EMIT_EXPR_SUCCESS_CONSTANT, EMIT_EXPR_SUCCESS);

	return	aclexpr_arithmetic_cast(
			DATA.m_id_ret
			, DATA.m_tyid_current 
			, DATA.m_literal_flag 
			, DATA.m_ret_literal
			, ret_id
			, c_type_req
			, wr_literal_opt
			, h_ctx);
}

