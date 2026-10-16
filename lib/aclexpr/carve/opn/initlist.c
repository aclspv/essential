#include "./initlist.h"

#include <aclexpr/carve/util.h>
#include <aclexpr/op.h>

#include <aclty.h>
#include <aclcur/common.h>
#include <aclcur/load/get.h>

#include <util/constant.h>
#include <util/emitx.h>
#include <util/wrdemit.h>
#include <util/iddef.h>

#include <ae2f/cc/switchctl.h>
#include <ae2f/pat/dbg.h>

#define CHECK unless(rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_OPCODE] == (aclspv_wrd_t)ACLEXPR_OPN_INITLIST) return c_lbl_continue

/** one 3-word OpConstantNull in the types section; valid for every member
 * type, so union padding needs no per-type zero construction. */
static aclspv_id_t s_mk_null(
		aclspv_id_t		c_ty,
		h_aclspv_ctx_t		h_ctx) {
	const aclspv_wrd_t ANCHOUR = h_ctx->m_count.m_types;
	const aclspv_id_t ID = h_ctx->m_id++;

	ae2f_ifezerr(h_ctx->m_count.m_types = aclutil_emit_wrd(
				&h_ctx->m_section.m_types
				, ANCHOUR + 2
				, 0))
		return 0;

	aclutil_get_wrd_of_vec(&h_ctx->m_section.m_types)[ANCHOUR + 0]
		= aclspv_opcode_to_wrd(SpvOpConstantNull, 2);
	aclutil_get_wrd_of_vec(&h_ctx->m_section.m_types)[ANCHOUR + 1]
		= c_ty;
	aclutil_get_wrd_of_vec(&h_ctx->m_section.m_types)[ANCHOUR + 2]
		= ID;

	return ID;
}

aclexpr_carve_lbl_t aclexpr_carve_opn_initlist(
		struct aclexpr_visit_data* const ae2f_restrict	rdwr_data,
		aclspv_wrd_t* const ae2f_restrict rdwr_prv_idval,
		aclexpr_opbincfg_extra* const ae2f_restrict rdwr_prv_extra,
		ae2f_unused h_scale_t			h_lst_scale,
		aclspv_wrd_t* const ae2f_restrict	rdwr_lst_scale_buf,
		aclexpr_opbincfg_extra* ae2f_restrict rdwr_lst_extra,
		aclutil_literal* ae2f_restrict		r_ltrl,
		aclexpr_carve_lbl_t			c_lbl_continue,
		aclexpr_carve_lbl_t			c_lbl_success,
		aclexpr_carve_lbl_t			c_lbl_err
		) { CHECK; {
	const aclspv_wrd_t	N0	= rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_NUMOPRND];
	aclspv_wrd_t		N	= N0;
	const aclspv_id_t	TY_RES	= rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_RESTYID];
	const aclty_t* ae2f_restrict P_TY;
	aclspv_id_t		TY_ELM	= 0;
	aclspv_wrd_t		EXPECTED = 0;
	aclspv_wrd_t		MASK = ACLUTIL_LITERAL_MASK_LITERAL;
	aclspv_wrd_t		i;
	/** local operand list: union pads append ids past the visited
	 * operands, but the scale's operand slots and extras share one
	 * buffer (extras = buf + END + NUMOPRND), so anything appended past
	 * the visited count would clobber extras[0]. keep pads here. */
	aclspv_id_t		OP[ACLCUR_OPX_OPRND_CAP];
	aclexpr_opbincfg_extra* ae2f_restrict XTRA = rdwr_lst_extra;

	/** opX holds at most ACLCUR_OPX_OPRND_CAP operands; empty lists carry no value here. */
	ae2f_ifnezerr_strict(N == 0 || N > ACLCUR_OPX_OPRND_CAP) {
		err_puts("[carve::opn::initlist] element count out of range");
		return c_lbl_err;
	}

	ae2f_ifezerr(TY_RES) {
		err_puts("[carve::opn::initlist] no result type");
		return c_lbl_err;
	}

	P_TY = aclty_find(TY_RES, CTX(rdwr_data));

	ae2f_ifezerr(P_TY) {
		err_puts("[carve::opn::initlist] result type not found");
		return c_lbl_err;
	}

	switch(P_TY->m_class) {
		case ACLTY_RESERVED:
		case ACLTY_FN:
		case ACLTY_FN_ARG:
		case ACLTY_PTR:
		case ACLTY_STRUCT_MBR:
		case ACLTY_DCR:
		default:
			err_prefix();
			err_call((void), fprintf(stderr
						, "[carve::opn::initlist] unsupported aggregate class (%u)\n"
						, (unsigned)P_TY->m_class));
			CTX(rdwr_data)->m_err = CTX(rdwr_data)->m_err
				? CTX(rdwr_data)->m_err
				: ACLSPV_COMPILE_UNHANDLED_CARVE;
			return c_lbl_err;

		case ACLTY_STRUCT:
			{
				/** struct OR union (unions also model as ACLTY_STRUCT):
				 * members are individually typed, so EXPECTED is the
				 * member count and the cast loop below resolves each
				 * operand's target type per member index. */
				EXPECTED = P_TY->m_info.m_struct.m_num_mbr;

				/** union: C90 takes a single initializer for the first
				 * member. pad the remaining members with null constants
				 * so the composite machinery below runs unchanged. the
				 * visit lane has already sized the scale for EXPECTED
				 * operand slots and extras.
				 *
				 * XTRA stays at rdwr_lst_extra (buf + END + NUMOPRND),
				 * which is where every child visit wrote its extra —
				 * for a single-element union that is the element's own
				 * slot. pad extras go to XTRA[i] (i >= 1), which the
				 * visit lane zeroed for M extras. the pad ids live in
				 * the local OP list, never in the scale buffer, so
				 * extras[0] survives untouched. */
				if((P_TY->m_field0 & ACLTY_STRUCT_FIELD0_UNION)
						&& N == 1 && EXPECTED > 1) {
					ae2f_ifnezerr_strict(EXPECTED > ACLCUR_OPX_OPRND_CAP) {
						err_puts("[carve::opn::initlist] union member count too large");
						return c_lbl_err;
					}

					for(i = 1; i < EXPECTED; ++i) {
						const aclspv_id_t TY_I
							= aclty_get_struct_mbr((aclty_t*)P_TY, i, CTX(rdwr_data))
							->m_info.m_struct_mbr.m_id_mbr;

						ae2f_ifezerr(TY_I) {
							err_puts("[carve::opn::initlist] union member type unresolved");
							return c_lbl_err;
						}

						OP[i] = s_mk_null(TY_I, CTX(rdwr_data));

						aclmemzero(&XTRA[i], sizeof(XTRA[i]));
						XTRA[i].m_type_original = TY_I;
						XTRA[i].m_mask_literal = ACLUTIL_LITERAL_MASK_CONSTANT;
					}

					N = EXPECTED;
				}
			}
			break;

		case ACLTY_VEC:
			{
				const struct aclutil_cxtp_veccfg_t V = aclty_get_veccfg(TY_RES, CTX(rdwr_data));
				TY_ELM		= V.m_ty_elm;
				EXPECTED	= V.m_num;
			}
			break;

		case ACLTY_ARR:
			{
				const aclcur_t LB = aclcur_get_load(
						P_TY->m_info.m_arr.m_num.m_id
						, CTX(rdwr_data)
						);

				TY_ELM = P_TY->m_info.m_arr.m_type;

				/** the array length must be a plain literal (not a
				 * spec-constant) to size the list. */
				ae2f_ifnezerr_strict(
						(LB.m_val_mask & ACLUTIL_LITERAL_MASK_LITERAL)
						!= ACLUTIL_LITERAL_MASK_LITERAL) {
					err_puts("[carve::opn::initlist] array length is not a literal");
					return c_lbl_err;
				}

				EXPECTED = (aclspv_wrd_t)LB.m_val_lit.m_u64;
			}
			break;
	}

	/** for non-struct aggregates TY_ELM is the single element type;
	 * structs keep it 0 and resolve per-member targets in the loop. */
	ae2f_ifnezerr_strict(P_TY->m_class != ACLTY_STRUCT) {
		ae2f_ifezerr(TY_ELM) {
			err_puts("[carve::opn::initlist] element type unresolved");
			return c_lbl_err;
		}
	}

	ae2f_ifnezerr_strict(EXPECTED != N) {
		err_prefix();
		err_call((void), fprintf(stderr
					, "[carve::opn::initlist] count mismatch: %u elements for a %u-wide aggregate\n"
					, (unsigned)N
					, (unsigned)EXPECTED));
		return c_lbl_err;
	}

	*rdwr_prv_idval = CTX(rdwr_data)->m_id++;

	/** cast every element into the aggregate's element type, in order.
	 * for structs/unions each member is individually typed. */
	for(i = 0; i < N0; ++i)
		OP[i] = rdwr_lst_scale_buf[ACLEXPR_OPBINCFG_END + i];

	for(i = 0; i < N; ++i) {
		const aclutil_literal OLD_LITERAL = XTRA[i].m_literal;
		const aclspv_id_t OLD_ID = OP[i];
		const aclspv_id_t TY_I = (P_TY->m_class == ACLTY_STRUCT)
			? aclty_get_struct_mbr((aclty_t*)P_TY, i, CTX(rdwr_data))->m_info.m_struct_mbr.m_id_mbr
			: TY_ELM;
		aclspv_id_t NEW_ID;

		/** EXPECTED == m_num_mbr was checked above, so i < m_num_mbr
		 * always; guard anyway — a 0 member type must never reach the
		 * cast. */
		ae2f_ifezerr(TY_I) {
			err_puts("[carve::opn::initlist] struct member type unresolved");
			return c_lbl_err;
		}

		switch(aclexpr_arithmetic_cast(
					OLD_ID
					, XTRA[i].m_type_original
					, XTRA[i].m_mask_literal
					, OLD_LITERAL
					, &NEW_ID
					, TY_I
					, &XTRA[i].m_literal
					, CTX(rdwr_data)
					)) {
			default:
			case ACLEXPR_RET_FAILURE:
			case ACLEXPR_RET_NOT_THE_CASE:
				err_prefix(a);
				err_call((void), fprintf(stderr
							, "[carve::opn::initlist] element %u cast failed\n"
							, (unsigned)i));
				return c_lbl_err;

			case ACLEXPR_RET_SUCCESS:
				MASK &= 0;
				ae2f_fallthrough;
			case ACLEXPR_RET_SUCCESS_CONSTANT:
				MASK &= 1;
				ae2f_fallthrough;
			case ACLEXPR_RET_SUCCESS_LITERAL:
				break;
		}

		OP[i] = NEW_ID;
	}

	/** a scalar braced initializer `int x = { 3 }`: the element IS the
	 * value (a 1-wide composite does not exist). */
	if(EXPECTED == 1 && aclty_simd_elmtype(TY_RES, rdwr_data->m_ctx)) {
		rdwr_prv_extra->m_type_original = TY_RES;
		rdwr_prv_extra->m_mask_literal = MASK;
		rdwr_prv_extra->m_literal = XTRA[0].m_literal;
		*rdwr_prv_idval = OP[0];
		*r_ltrl = rdwr_prv_extra->m_literal;
		return c_lbl_success;
	}

	rdwr_prv_extra->m_type_original = TY_RES;

	if(MASK & ACLUTIL_LITERAL_CONSTANT) {
		/** all elements foldable: emit OpConstantComposite straight into
		 * the types section. a composite is never a scalar literal, so
		 * the mask is CONSTANT only (0x1). */
		const aclspv_wrd_t ANCHOUR = CTX(rdwr_data)->m_count.m_types;

		ae2f_ifezerr(CTX(rdwr_data)->m_count.m_types = aclutil_emit_wrd(
					&CTX(rdwr_data)->m_section.m_types
					, (aclspv_wrd_t)(ANCHOUR + 2 + N)
					, 0)) {
			err_puts("[carve::opn::initlist] emit OpConstantComposite");
			return c_lbl_err;
		}

		aclutil_get_wrd_of_vec(&CTX(rdwr_data)->m_section.m_types)[ANCHOUR + 0]
			= aclspv_opcode_to_wrd(SpvOpConstantComposite, N + 2);
		aclutil_get_wrd_of_vec(&CTX(rdwr_data)->m_section.m_types)[ANCHOUR + 1]
			= TY_RES;
		aclutil_get_wrd_of_vec(&CTX(rdwr_data)->m_section.m_types)[ANCHOUR + 2]
			= *rdwr_prv_idval;

		for(i = 0; i < N; ++i) {
			aclutil_get_wrd_of_vec(&CTX(rdwr_data)->m_section.m_types)[ANCHOUR + 3 + i]
				= OP[i];
		}

		rdwr_prv_extra->m_mask_literal = ACLUTIL_LITERAL_MASK_CONSTANT;
		*r_ltrl = rdwr_prv_extra->m_literal;
		return c_lbl_success;
	}

	/** any runtime element: build an OpCompositeConstruct opX node so
	 * aclcur_emit writes it into m_fnimpl in program order. */
	ae2f_ifezerr(*rdwr_prv_idval = aclcur_cfg_opx(
				*rdwr_prv_idval
				, SpvOpCompositeConstruct
				, TY_RES
				, OP
				, N
				, CTX(rdwr_data)
				)) {
		err_puts("[carve::opn::initlist] aclcur_cfg_opx OpCompositeConstruct");
		return c_lbl_err;
	}

	rdwr_prv_extra->m_mask_literal = ACLUTIL_LITERAL_MASK_RT;
	return c_lbl_success;
} ae2f_unreachable(); }

#undef	CHECK
