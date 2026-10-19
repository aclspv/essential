#include "./simd.h"
#include <aclty.h>


aclspv_id_t aclty_simd_elmtype(aclspv_id_t c, h_aclspv_ctx_t h_ctx) {
	unless(c) return 0;
	ae2f_ifnezerr_strict(c == ACLSPV_ID_VOID) return 0;

	if(c < (aclspv_id_t)ACLSPV_ID_END) {
		return c;
	}

	{
		aclty_t* ae2f_restrict p = aclty_find(c, h_ctx);

		ae2f_ifezerr(p)
			return 0;

		switch(p->m_class) {
			default:
				ae2f_unreachable();

			case ACLTY_STRUCT:
			case ACLTY_ARR:
			case ACLTY_PTR:
			case ACLTY_FN:
			case ACLTY_FN_ARG:
			case ACLTY_STRUCT_MBR:
			case ACLTY_DCR:
			case ACLTY_RESERVED:
				return 0;

			case ACLTY_VEC:
				return p->m_info.m_vec.m_type;
		}

		ae2f_unreachable();
	}
}

aclspv_id_t aclty_simd_chxg_cls(aclspv_id_t c, aclspv_id_t c_class, h_aclspv_ctx_t h) {
	ae2f_ifezerr(c)		return 0;
	ae2f_ifezerr(c_class)	return 0;
	ae2f_ifezerr(c_class)	return 0;
	ae2f_ifnezerr_strict(c_class >= (aclspv_id_t)ACLSPV_ID_END) return 0;
	ae2f_ifnezerr_strict(c == ACLSPV_ID_VOID) return 0;
	ae2f_ifnezerr_strict(c_class == ACLSPV_ID_VOID) return 0;

	if(c < (aclspv_id_t)ACLSPV_ID_END) {
		return c_class;	/** c_class */
	}

	{
		aclty_t* ae2f_restrict p = aclty_find(c, h);

		ae2f_ifezerr(p)
			return 0;

		switch(p->m_class) {
			default:
				ae2f_unreachable();

			case ACLTY_STRUCT:
			case ACLTY_ARR:
			case ACLTY_PTR:
			case ACLTY_FN:
			case ACLTY_FN_ARG:
			case ACLTY_STRUCT_MBR:
			case ACLTY_DCR:
			case ACLTY_RESERVED:
				return 0;

			case ACLTY_VEC:
				return aclty_mk_vec(
						c_class
						, p->m_info.m_vec.m_num
						, h
						);
		}

		ae2f_unreachable();
	}
}
