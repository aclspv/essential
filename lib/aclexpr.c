/** @file expr.c */

#include <aclexpr.h>
#include <aclexpr/op.h>

#include <ae2f/bll.h>
#include <ae2f/pat/dbg.h>

#include <util/scale.h>
#include <util/emitx.h>
#include <util/log.h>
#include <util/iddef.h>
#include <util/visitfromroot.h>

enum ACLEXPR_RET_ aclspv_get_expr2(
		aclspv_id_t* ae2f_restrict const	ret_id
		, aclspv_id_t* ae2f_restrict const	r_type_opt
		, const CXCursor			c_cur
		, aclutil_literal* ae2f_restrict		wr_literal_opt
		, aclutil_fnctx* ae2f_restrict			h_fnctx
		) {
	aclspv_wrd_t* ae2f_restrict SCALE_BOOT;
	struct aclexpr_visit_data	DATA;
	const h_aclspv_ctx_t		CTX = h_fnctx->m_ctx;

	ae2f_ifezerr(CTX)	return ACLEXPR_RET_FAILURE;

	DATA.m_ctx		= CTX;				/* context. must be pointer to context. */
	DATA.m_fnctx		= h_fnctx;			/* context. must be pointer to context. */
	DATA.m_exprcount	= 0;				/* stack count. must be 0. */
	DATA.m_id_ret		= 0;				/* result value id */
	DATA.m_literal_flag	= ACLUTIL_LITERAL_MASK_LITERAL;	/* see enum UTIL_LITERAL_MASK_ */
	DATA.m_tyid_current	= 0;				/* type casted */
	DATA.m_num_cmdcounter	= 1;				/* count for operand */

	ae2f_expected_but_else(CTX->m_tmp.m_v1.m_p)
		return ACLEXPR_RET_FAILURE;

	ae2f_expected_but_else(init_scale(&CTX->m_tmp.m_v1
				, aclutil_count_to_sz(ACLEXPR_OPBINCFG_END) + sizeof(aclspv_wrd_t)
				+ sizeof(aclexpr_opbincfg_extra)
				))
		return ACLEXPR_RET_FAILURE;

	SCALE_BOOT = get_buf_from_scale2(aclspv_wrd_t, &CTX->m_tmp.m_v1, *get_last_scale_from_vec(&CTX->m_tmp.m_v1));
	ae2fpat_assertassume(SCALE_BOOT);
	ae2fpat_assertassume(!get_last_scale_from_vec(&CTX->m_tmp.m_v1)->m_id);

	SCALE_BOOT[ACLEXPR_OPBINCFG_NUMOPRND]		= 1;
	SCALE_BOOT[ACLEXPR_OPBINCFG_RESTYID]		= 0;
	SCALE_BOOT[ACLEXPR_OPBINCFG_RESID]		= 0;
	SCALE_BOOT[ACLEXPR_OPBINCFG_OPCODE]		= 0;
#define INIT_EXTRA ((aclexpr_opbincfg_extra* ae2f_restrict)(void* ae2f_restrict)(&SCALE_BOOT[ACLEXPR_OPBINCFG_END + 1]))

	INIT_EXTRA->m_mask_literal		= 3;
	INIT_EXTRA->m_type_original		= 0;
	INIT_EXTRA->m_literal.m_api_uintmax	= 0;

	((umax* ae2f_restrict)CTX->m_tmp.m_v2.m_p)[0] = 0;

	dbg_prefix(a);
	dbg_call(fprintf, (stderr
				, "Initialising first constant sector %u[%p]\n"
				, SCALE_BOOT[1], (void*)(SCALE_BOOT + 1))
		);

	(void)aclutil_visitfromroot(c_cur, aclexpr_visit, &DATA);
	ae2f_ifnezerr(CTX->m_err) return ACLEXPR_RET_FAILURE;

	if(wr_literal_opt)
		*wr_literal_opt = INIT_EXTRA->m_literal;

	if(r_type_opt)
		*r_type_opt = DATA.m_tyid_current;

	ae2fpat_assertassume_strict(ret_id);
	*ret_id = DATA.m_id_ret;

	return		DATA.m_literal_flag & ACLUTIL_LITERAL_LITERAL ? ACLEXPR_RET_SUCCESS_LITERAL
		: 	DATA.m_literal_flag & ACLUTIL_LITERAL_CONSTANT ? ACLEXPR_RET_SUCCESS_CONSTANT
		: 	ACLEXPR_RET_SUCCESS;

#undef	INIT_EXTRA
}

enum ACLEXPR_RET_ aclexpr_eval_btcast(
		aclspv_id_t* ae2f_restrict const ret_id
		, const aclspv_id_t c_type_req
		, const CXCursor c_cur
		, aclutil_literal* ae2f_restrict	wr_literal_opt
		, aclutil_fnctx* ae2f_restrict h_fnctx
		) 
{
	aclspv_wrd_t* ae2f_restrict SCALE_BOOT;
	struct aclexpr_visit_data	DATA;
	const h_aclspv_ctx_t		CTX = h_fnctx->m_ctx;

	ae2f_ifezerr(h_fnctx)	return ACLEXPR_RET_FAILURE;

	DATA.m_ctx		= h_fnctx->m_ctx;		/* context. must be pointer to context. */
	DATA.m_fnctx		= h_fnctx;			/* context. must be pointer to context. */
	DATA.m_exprcount	= 0;				/* stack count. must be 0. */
	DATA.m_id_ret		= 0;				/* result value id */
	DATA.m_literal_flag	= ACLUTIL_LITERAL_MASK_LITERAL;	/* see enum UTIL_LITERAL_MASK_ */
	DATA.m_tyid_current	= 0;				/* type casted */
	DATA.m_num_cmdcounter	= 1;				/* count for operand */

	ae2f_expected_but_else(init_scale(&CTX->m_tmp.m_v1
				, aclutil_count_to_sz(ACLEXPR_OPBINCFG_END) + sizeof(aclspv_wrd_t)
				+ sizeof(aclexpr_opbincfg_extra)
				))
		return ACLEXPR_RET_FAILURE;

	SCALE_BOOT = get_buf_from_scale2(aclspv_wrd_t, &CTX->m_tmp.m_v1, *get_last_scale_from_vec(&CTX->m_tmp.m_v1));
	ae2fpat_assertassume_strict(!!SCALE_BOOT);
	ae2fpat_assertassume_strict(!get_last_scale_from_vec(&CTX->m_tmp.m_v1)->m_id);

	SCALE_BOOT[ACLEXPR_OPBINCFG_NUMOPRND]		= 1;
	SCALE_BOOT[ACLEXPR_OPBINCFG_RESTYID]		= 0;
	SCALE_BOOT[ACLEXPR_OPBINCFG_RESID]			= 0;
	SCALE_BOOT[ACLEXPR_OPBINCFG_OPCODE]		= 0;
#define INIT_EXTRA ((aclexpr_opbincfg_extra* ae2f_restrict)(void* ae2f_restrict)(&SCALE_BOOT[ACLEXPR_OPBINCFG_END + 1]))

	INIT_EXTRA->m_mask_literal		= 3;
	INIT_EXTRA->m_type_original		= 0;
	INIT_EXTRA->m_literal.m_api_uintmax	= 0;

#undef	INIT_EXTRA

	ae2f_ifnezerr(s_aclmem_cgrow2(
				&CTX->m_tmp.m_v2
				, 1, sizeof(aclspv_wrd_t)
				, ACLSPV_COMPILE_ALLOC_FAILED_EXPR))
		return ACLEXPR_RET_FAILURE;
	((aclspv_wrd_t* ae2f_restrict)CTX->m_tmp.m_v2.m_p)[0] = 0;

	dbg_prefix(a);
	dbg_call(fprintf
			, (stderr
				, "Initialising first constant sector %u[%p]\n"
				, SCALE_BOOT[1]
				, (void*)(SCALE_BOOT + 1))
		);

	(void)aclutil_visitfromroot(c_cur, aclexpr_visit, &DATA);
	ae2f_ifnezerr(CTX->m_err) return ACLEXPR_RET_FAILURE;

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
				, CTX);

	ae2f_expected_but_else(aclid_is_number(c_type_req) 
			&& aclid_is_number(DATA.m_tyid_current)) {
		return ACLEXPR_RET_NOT_THE_CASE;
	}

	if(DATA.m_literal_flag & ACLUTIL_LITERAL_LITERAL) {
		if(wr_literal_opt)
			*wr_literal_opt = DATA.m_ret_literal;

		return aclexpr_arithmetic_cast_literal(
				c_type_req
				, DATA.m_ret_literal
				, ret_id
				, c_type_req
				, wr_literal_opt
				, CTX
				);
	}

	/** NON-LITERAL */
	{
		const aclspv_id_t	TYPE_SRC = aclid_getty_uint(
				aclid_get_bit_width(DATA.m_tyid_current)
				);

		const aclspv_id_t	TYPE_DST = aclid_getty_uint(aclid_get_bit_width(c_type_req));

		const aclspv_id_t	VAL_SRC = CTX->m_id;
		typedef const aclspv_id_t* ae2f_restrict ptr_id_t;

		ptr_id_t VAL_SRC_PTR, TYPE_SRC_PTR;
		aclspv_id_t	VAL_DST;

		ae2f_expected_but_else(aclid_mk_default_id(TYPE_DST, CTX)
				&& aclid_mk_default_id(TYPE_SRC, CTX)) {
			return ACLEXPR_RET_FAILURE;
		}

		unless(aclid_is_int(DATA.m_tyid_current)) {
			++CTX->m_id;
			ae2f_expected_but_else(CTX->m_count.m_fnimpl = aclutil_emitx_4(
						&CTX->m_section.m_fnimpl
						, CTX->m_count.m_fnimpl
						, SpvOpBitcast
						, TYPE_SRC
						, VAL_SRC
						, DATA.m_id_ret 
						)) {
				return ACLEXPR_RET_FAILURE;
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
					, CTX
					)) {
			default:
			case ACLEXPR_RET_NOT_THE_CASE:
				return ACLEXPR_RET_NOT_THE_CASE;
			case ACLEXPR_RET_FAILURE:
				return ACLEXPR_RET_FAILURE;

			case ACLEXPR_RET_SUCCESS:
			case ACLEXPR_RET_SUCCESS_CONSTANT:
			case ACLEXPR_RET_SUCCESS_LITERAL:
				break;
		}

		ae2fpat_assertassume(aclid_is_uint(TYPE_DST));

		unless(aclid_is_int(c_type_req)) {
			ae2f_expected_but_else(CTX->m_count.m_fnimpl = aclutil_emitx_4(
						&CTX->m_section.m_fnimpl
						, CTX->m_count.m_fnimpl
						, SpvOpBitcast
						, c_type_req
						, *ret_id = CTX->m_id++
						, VAL_DST
						)) {
				return ACLEXPR_RET_FAILURE;
			}

			return ACLEXPR_RET_SUCCESS;
		} else {
			return aclexpr_arithmetic_cast_non_literal(
					VAL_DST
					, TYPE_DST
					, 0
					, ret_id
					, c_type_req
					, CTX
					);
		}
	}

	ae2f_unreachable();
}

enum ACLEXPR_RET_ aclexpr_eval(
		aclspv_id_t* ae2f_restrict const ret_id
		, const aclspv_id_t c_type_req
		, const CXCursor c_cur
		, aclutil_literal* ae2f_restrict	wr_literal_opt
		, aclutil_fnctx* ae2f_restrict h_fnctx
		) {
	aclspv_wrd_t* ae2f_restrict SCALE_BOOT;
	struct aclexpr_visit_data	DATA;
	h_aclspv_ctx_t			CTX = h_fnctx->m_ctx;

	DATA.m_ctx		= CTX;				/* context. must be pointer to context. */
	DATA.m_fnctx		= h_fnctx;			/* context. must be pointer to context. */
	DATA.m_exprcount	= 0;				/* stack count. must be 0. */
	DATA.m_id_ret		= 0;				/* result value id */
	DATA.m_literal_flag	= ACLUTIL_LITERAL_MASK_LITERAL;	/* see enum UTIL_LITERAL_MASK_ */
	DATA.m_tyid_current	= 0;				/* type casted */
	DATA.m_num_cmdcounter	= 1;				/* count for operand */

	ae2f_expected_but_else(init_scale(&CTX->m_tmp.m_v1
				, aclutil_count_to_sz(ACLEXPR_OPBINCFG_END) + sizeof(aclspv_wrd_t)
				+ sizeof(aclexpr_opbincfg_extra)
				))
		return ACLEXPR_RET_FAILURE;

	SCALE_BOOT = get_buf_from_scale2(aclspv_wrd_t, &CTX->m_tmp.m_v1, *get_last_scale_from_vec(&CTX->m_tmp.m_v1));
	ae2fpat_assertassume_strict(!!SCALE_BOOT);
	ae2fpat_assertassume_strict(!get_last_scale_from_vec(&CTX->m_tmp.m_v1)->m_id);

	SCALE_BOOT[ACLEXPR_OPBINCFG_NUMOPRND]		= 1;
	SCALE_BOOT[ACLEXPR_OPBINCFG_RESTYID]		= 0;
	SCALE_BOOT[ACLEXPR_OPBINCFG_RESID]			= 0;
	SCALE_BOOT[ACLEXPR_OPBINCFG_OPCODE]		= 0;
#define INIT_EXTRA ((aclexpr_opbincfg_extra* ae2f_restrict)(void* ae2f_restrict)(&SCALE_BOOT[ACLEXPR_OPBINCFG_END + 1]))

	INIT_EXTRA->m_mask_literal		= 3;
	INIT_EXTRA->m_type_original		= 0;
	INIT_EXTRA->m_literal.m_api_uintmax	= 0;

#undef	INIT_EXTRA

	ae2f_ifnezerr(s_aclmem_cgrow2(
				&CTX->m_tmp.m_v2
				, 1, sizeof(aclspv_wrd_t)
				, ACLSPV_COMPILE_ALLOC_FAILED_EXPR))
		return ACLEXPR_RET_FAILURE;

	((aclspv_wrd_t* ae2f_restrict)CTX->m_tmp.m_v2.m_p)[0] = 0;

	dbg_prefix(a);
	dbg_call(fprintf
			, (stderr
				, "Initialising first constant sector %u[%p]\n"
				, SCALE_BOOT[1]
				, (void*)(SCALE_BOOT + 1))
		);

	(void)aclutil_visitfromroot(c_cur, aclexpr_visit, &DATA);
	ae2f_ifnezerr(CTX->m_err) return ACLEXPR_RET_FAILURE;

	unless(c_type_req)
		return ae2f_bll_sel_bool1(uint_fastenum_t, DATA.m_literal_flag & 2, ACLEXPR_RET_SUCCESS_LITERAL, 0u)
		| ae2f_bll_sel_bool1(uint_fastenum_t, DATA.m_literal_flag & 1, ACLEXPR_RET_SUCCESS_CONSTANT, ACLEXPR_RET_SUCCESS);

	return	aclexpr_arithmetic_cast(
			DATA.m_id_ret
			, DATA.m_tyid_current 
			, DATA.m_literal_flag 
			, DATA.m_ret_literal
			, ret_id
			, c_type_req
			, wr_literal_opt
			, CTX);
}

