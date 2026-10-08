#include "./get.h"
#include <aclutil/emitx.h>
#include <ae2f/cc/pause.h>

/** c_load might be somewhere in constant? */
static ae2f_ccpure aclcur_t s_get_const(
		aclspv_id_t c_load,
		h_aclspv_ctx_t h_ctx)	
{{
	aclcur_t r = {0, };
	const aclspv_wrd_t* ae2f_restrict op = aclutil_get_wrd_of_vec(&h_ctx->m_section.m_types);
	const aclspv_wrd_t* const ae2f_restrict op_end = op + h_ctx->m_count.m_types;

	r.m_flag0 |= aclcur_flag0(ACLCUR_FLAG0_VALUE);

	while(op < op_end) unless(op[2] == c_load) goto SKIP; else switch(0xFF & *op) {
		default:
SKIP:
			op += (*op >> 16u);
			ae2fcc_pause();
			break;

			ae2f_unreachable();

V2T1:
			r.m_class	= ACLCUR_LOAD;
			r.m_val		= op[2];
			r.m_ty		= op[1];
			return r;

		case SpvOpConstant:
			/** constant being double words */
			if(((*op >> 16) - 3) < 3) {
				r.m_val_mask		= ACLUTIL_LITERAL_MASK_LITERAL;
				r.m_val_lit.m_u64	= op[3];
				if(((*op >> 16) - 3) == 2) {
					r.m_val_lit.m_u64	|= ((aclspv_wrd2_t)op[4] << 32);
				}
			} else {
				r.m_val_mask = ACLUTIL_LITERAL_CONSTANT;
			}
			goto V2T1;

		case SpvOpConstantTrue:
			r.m_val_lit.m_api_uintmax = 1;
			r.m_val_mask = ACLUTIL_LITERAL_MASK_LITERAL;
			goto V2T1;

		case SpvOpConstantFalse:
		case SpvOpConstantNull:
			r.m_val_lit.m_api_uintmax = 0;
			r.m_val_mask = ACLUTIL_LITERAL_MASK_LITERAL;
			goto V2T1;

		case SpvOpConstantComposite:
		case SpvOpSpecConstant:
		case SpvOpSpecConstantTrue:
		case SpvOpSpecConstantFalse:
		case SpvOpSpecConstantComposite:
		case SpvOpSpecConstantOp:
			r.m_val_mask = ACLUTIL_LITERAL_CONSTANT;
			goto V2T1;
	}

	return r;
 }}

ae2f_ccpure aclcur_t aclcur_get_load(
		aclspv_id_t c_load,
		h_aclspv_ctx_t h_ctx) {
	h_aclcur_t p_load = aclcur_get_node(c_load, h_ctx);

	if(p_load) {
#if	__ACLSPV_CHECK_EXTRA
#define	s_assume(c)	ae2f_ifezerr((c)) { aclcur_t r = {0, }; err_puts("assume failed: " #c); return r; }
#else
#define s_assume(c)	ae2f_assume((c))
#endif
		s_assume(p_load->m_val);
		s_assume(p_load->m_flag0 & aclcur_flag0(ACLCUR_FLAG0_VALUE));
		return *p_load;
	}

	{
		aclcur_t r = s_get_const(c_load, h_ctx);

		/**
		 * raw id fallback: the id was allocated but is backed by no aclcur
		 * node nor an m_types constant -- e.g. a value emitted directly
		 * into a section (see eval's bitcast twin). emit as a raw operand.
		 * */
		unless(r.m_val)
			r.m_val = c_load;

		return r;
	}
}
