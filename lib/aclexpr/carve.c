#include "./carve.h"
#include "./carve/util.h"
#include "./carve/op1/arithmetic_cast.h"
#include "./carve/op1/deref.h"
#include "./carve/op1/incr.h"
#include "./carve/op1/memberread.h"
#include "./carve/op1/subscript.h"
#include "./carve/op1/glslmath.h"
#include "./carve/op1/not.h"
#include "./carve/op3/cond.h"
#if	__ACLSPV_MATH_NR
#include "./carve/op1/rsqrtf_nr.h"
#endif
#include "./carve/opn/initlist.h"
#include "./carve/opn/shuffle.h"
#include "./carve/op2/arithmetic.h"
#include "./carve/op2/cmp.h"
#include "./carve/op2/shift.h"
#include "./carve/op2/assign.h"
#include "./carve/op2/derefwrite.h"
#include "./carve/op2/memberwrite.h"
#include "./carve/op2/bool0.h"
#include "./carve/op2/bool1.h"
#include "./carve/op2/comma.h"

#include <aclexpr.h>

#include <aclutil/wrdemit.h>
#include <util/scale.h>

#include <ae2f/cgoto.h>
#include <ae2f/pat/dbg.h>

aclspv_bool_t aclexpr_carve(
		struct aclexpr_visit_data* const ae2f_restrict	rdwr_data,
		aclspv_wrd_t* const ae2f_restrict rdwr_prv_idval,
		aclexpr_opbincfg_extra* const ae2f_restrict rdwr_prv_extra
		) {
	h_scale_t LST_SCALE				= get_last_scale_from_vec(&CMDSTCK_SCALE(CTX(rdwr_data)));
	aclspv_wrd_t* ae2f_restrict LST_SCALE_BUF	= get_buf_from_scale2(
			aclspv_wrd_t
			, &CMDSTCK_SCALE(CTX(rdwr_data))
			, *LST_SCALE
			);
	aclutil_literal* ae2f_restrict RET_LITERAL
		= &RET_LITERAL_REF(rdwr_data);

	aclspv_wrd_t	NUM_OPRND;

	aclexpr_opbincfg_extra* ae2f_restrict LST_EXTRA;

	ae2fpat_assertassume(LST_SCALE_BUF);
	ae2fpat_assertassume(rdwr_prv_extra);
	ae2fpat_assertassume(rdwr_prv_idval);

	NUM_OPRND = LST_SCALE_BUF[ACLEXPR_OPBINCFG_NUMOPRND];
	LST_EXTRA = (void* ae2f_restrict)(LST_SCALE_BUF + NUM_OPRND + ACLEXPR_OPBINCFG_END);

	dbg_prefix();
	dbg_call((void), fprintf(stderr, "[aclexpr::carve] oprnd %u opcode %u\n"
				, (unsigned)LST_SCALE_BUF[ACLEXPR_OPBINCFG_NUMOPRND]
				, (unsigned)LST_SCALE_BUF[ACLEXPR_OPBINCFG_OPCODE]));


	switch((enum ACLEXPR_OP_)LST_SCALE_BUF[ACLEXPR_OPBINCFG_OPCODE]) {
		case ACLEXPR_OPN_INITLIST:
			return (aclspv_bool_t)ae2f_expected_not_strict(aclexpr_carve_opn_initlist(
						rdwr_data
						, rdwr_prv_idval
						, rdwr_prv_extra
						, LST_SCALE
						, LST_SCALE_BUF
						, LST_EXTRA
						, RET_LITERAL
						));

		case ACLEXPR_OPN_SHUFFLE:
			return (aclspv_bool_t)ae2f_expected_not_strict(aclexpr_carve_opn_shuffle(
						rdwr_data
						, rdwr_prv_idval
						, rdwr_prv_extra
						, LST_SCALE
						, LST_SCALE_BUF
						, LST_EXTRA
						, RET_LITERAL
						));

		case ACLEXPR_OP2_ADD:
		case ACLEXPR_OP2_ADDASS:
		case ACLEXPR_OP2_SUB:
		case ACLEXPR_OP2_SUBASS:
		case ACLEXPR_OP2_MUL:
		case ACLEXPR_OP2_MULASS:
		case ACLEXPR_OP2_REM:
		case ACLEXPR_OP2_REMASS:
		case ACLEXPR_OP2_DIV:
		case ACLEXPR_OP2_DIVASS:
			return (aclspv_bool_t)ae2f_expected_not_strict(aclexpr_carve2_arithmetic(
						rdwr_data
						, rdwr_prv_idval
						, rdwr_prv_extra
						, LST_SCALE_BUF
						, LST_EXTRA
						, RET_LITERAL
						));

		case ACLEXPR_OP2_SLT:
		case ACLEXPR_OP2_SLE:
		case ACLEXPR_OP2_SGT:
		case ACLEXPR_OP2_SGE:
		case ACLEXPR_OP2_EQ:
		case ACLEXPR_OP2_NE:
			return (aclspv_bool_t)ae2f_expected_not_strict(aclexpr_carve2_cmp(
						rdwr_data
						, rdwr_prv_idval
						, rdwr_prv_extra
						, LST_SCALE_BUF
						, LST_EXTRA
						, RET_LITERAL
						));

		case ACLEXPR_OP2_AND_L:
		case ACLEXPR_OP2_OR_L:
			/** the before-visitor skipped the branch exactly when the RHS
			 * had no side effect. the evaluated RHS carries NOPURE iff it
			 * did, so: clear == flattened, set == branch/phi was created.
			 * the left operand is always evaluated and is irrelevant. */
			if(!(LST_EXTRA[1].m_state0 & ACLEXPR_OPCFGX_STATE0_NOPURE)) {
				return (aclspv_bool_t)ae2f_expected_not_strict(aclexpr_carve2_bool0(
							rdwr_data
							, rdwr_prv_idval
							, rdwr_prv_extra
							, LST_SCALE
							, LST_SCALE_BUF
							, LST_EXTRA
							, RET_LITERAL
							));
			}

			return (aclspv_bool_t)ae2f_expected_not_strict(aclexpr_carve2_bool1(
						rdwr_data
						, rdwr_prv_idval
						, rdwr_prv_extra
						, LST_SCALE_BUF
						, LST_EXTRA
						));

		case ACLEXPR_OP2_AND_A:
		case ACLEXPR_OP2_OR_A:
		case ACLEXPR_OP2_XOR_A:
		case ACLEXPR_OP2_AND:
		case ACLEXPR_OP2_OR:
		case ACLEXPR_OP2_XOR:
			return (aclspv_bool_t)ae2f_expected_not_strict(aclexpr_carve2_bool0(
						rdwr_data
						, rdwr_prv_idval
						, rdwr_prv_extra
						, LST_SCALE
						, LST_SCALE_BUF
						, LST_EXTRA
						, RET_LITERAL
						));

		case ACLEXPR_OP2_SHL:
		case ACLEXPR_OP2_SHLASS:
		case ACLEXPR_OP2_SHR:
		case ACLEXPR_OP2_SHRASS:
			return (aclspv_bool_t)ae2f_expected_not_strict(aclexpr_carve2_shift(
						rdwr_data
						, rdwr_prv_idval
						, rdwr_prv_extra
						, LST_SCALE_BUF
						, LST_EXTRA
						));

		case ACLEXPR_OP2_ASSIGN:
			return (aclspv_bool_t)ae2f_expected_not_strict(aclexpr_carve2_assign(
						rdwr_data
						, rdwr_prv_idval
						, rdwr_prv_extra
						, LST_SCALE_BUF
						, LST_EXTRA
						, RET_LITERAL
						));

		case ACLEXPR_OP2_COMMA:
			return (aclspv_bool_t)ae2f_expected_not_strict(aclexpr_carve2_comma(
						rdwr_data
						, rdwr_prv_idval
						, rdwr_prv_extra
						, LST_SCALE_BUF
						, LST_EXTRA
						, RET_LITERAL
						));

		case ACLEXPR_OP1_ADDRESS:
			return (aclspv_bool_t)ae2f_expected_not_strict(aclexpr_carve1_address(
						rdwr_data
						, rdwr_prv_idval
						, rdwr_prv_extra
						, LST_SCALE_BUF
						, LST_EXTRA
						));

		case ACLEXPR_OP1_DEREF:
			return (aclspv_bool_t)ae2f_expected_not_strict(aclexpr_carve1_deref(
						rdwr_data
						, rdwr_prv_idval
						, rdwr_prv_extra
						, LST_EXTRA
						));

		case ACLEXPR_OP1_NOT:
			return (aclspv_bool_t)ae2f_expected_not_strict(aclexpr_carve1_not(
						rdwr_data
						, rdwr_prv_idval
						, rdwr_prv_extra
						, LST_SCALE_BUF
						, LST_EXTRA
						, RET_LITERAL
						));

		case ACLEXPR_OP1_BITNOT:
			return (aclspv_bool_t)ae2f_expected_not_strict(aclexpr_carve1_bitnot(
						rdwr_data
						, rdwr_prv_idval
						, rdwr_prv_extra
						, LST_SCALE_BUF
						, LST_EXTRA
						, RET_LITERAL
						));

		case ACLEXPR_OP2_DEREFWRITE:
			return (aclspv_bool_t)ae2f_expected_not_strict(aclexpr_carve2_derefwrite(
						rdwr_data
						, rdwr_prv_idval
						, rdwr_prv_extra
						, LST_SCALE_BUF
						, LST_EXTRA
						, RET_LITERAL
						));

		case ACLEXPR_OP2_SUBSCRIPT:
			return (aclspv_bool_t)ae2f_expected_not_strict(aclexpr_carve2_subscript(
						rdwr_data
						, rdwr_prv_idval
						, rdwr_prv_extra
						, LST_SCALE_BUF
						, LST_EXTRA
						, RET_LITERAL
						));

		case ACLEXPR_OP2_SUBSCRIPTWRITE:
			return (aclspv_bool_t)ae2f_expected_not_strict(aclexpr_carve2_subscriptwrite(
						rdwr_data
						, rdwr_prv_idval
						, rdwr_prv_extra
						, LST_SCALE_BUF
						, LST_EXTRA
						, RET_LITERAL
						));

		case ACLEXPR_OP1_POSTDEC:
		case ACLEXPR_OP1_POSTINC:
		case ACLEXPR_OP1_PREDEC:
		case ACLEXPR_OP1_PREINC:
			return (aclspv_bool_t)ae2f_expected_not_strict(aclexpr_carve1_incr(
						rdwr_data
						, rdwr_prv_idval
						, rdwr_prv_extra
						, LST_SCALE_BUF
						, LST_EXTRA
						, RET_LITERAL
						));

		case ACLEXPR_OP1_EXPLICIT_CAST:
			return (aclspv_bool_t)ae2f_expected_not_strict(aclexpr_carve1_arithmetic_cast(
						rdwr_data
						, rdwr_prv_idval
						, rdwr_prv_extra
						, LST_SCALE_BUF
						, LST_EXTRA
						, RET_LITERAL
						));

		case ACLEXPR_OP1_MEMBERREAD:
			return (aclspv_bool_t)ae2f_expected_not_strict(aclexpr_carve1_memberread(
						rdwr_data
						, rdwr_prv_idval
						, rdwr_prv_extra
						, LST_SCALE_BUF
						, LST_EXTRA
						));

		case ACLEXPR_OP2_MEMBERWRITE:
			return (aclspv_bool_t)ae2f_expected_not_strict(aclexpr_carve2_memberwrite(
						rdwr_data
						, rdwr_prv_idval
						, rdwr_prv_extra
						, LST_SCALE_BUF
						, LST_EXTRA
						, RET_LITERAL
						));

		case ACLEXPR_OP1_SIN:
			return (aclspv_bool_t)ae2f_expected_not_strict(aclexpr_carve1_sin(
						rdwr_data
						, rdwr_prv_idval
						, rdwr_prv_extra
						, LST_SCALE_BUF
						, LST_EXTRA
						, RET_LITERAL
						));

		case ACLEXPR_OP1_COS:
			return (aclspv_bool_t)ae2f_expected_not_strict(aclexpr_carve1_cos(
						rdwr_data
						, rdwr_prv_idval
						, rdwr_prv_extra
						, LST_SCALE_BUF
						, LST_EXTRA
						, RET_LITERAL
						));

		case ACLEXPR_OP1_TAN:
			return (aclspv_bool_t)ae2f_expected_not_strict(aclexpr_carve1_tan(
						rdwr_data
						, rdwr_prv_idval
						, rdwr_prv_extra
						, LST_SCALE_BUF
						, LST_EXTRA
						, RET_LITERAL
						));

		case ACLEXPR_OP3_COND:
			return (aclspv_bool_t)ae2f_expected_not_strict(aclexpr_carve3_cond(
						rdwr_data
						, rdwr_prv_idval
						, rdwr_prv_extra
						, LST_SCALE_BUF
						, LST_EXTRA
						, RET_LITERAL
						));

#if	__ACLSPV_MATH_NR
		case ACLEXPR_OP1_RSQRTF_NR:
			return (aclspv_bool_t)ae2f_expected_not_strict(aclexpr_carve_op1_rsqrtf_nr(
						rdwr_data
						, rdwr_prv_idval
						, rdwr_prv_extra
						, LST_SCALE_BUF
						, LST_EXTRA
						, RET_LITERAL
						));

#endif

		default:
			ae2f_unreachable();
	}

	ae2f_unreachable();
}

