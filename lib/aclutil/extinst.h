#ifndef	aclutil_extinst_h
#define	aclutil_extinst_h

#include <aclspv.h>

/** id of the `c_name` OpExtInstImport, adding it on first use; 0 when
 * failed.
 *
 * same policy as aclutil_add_extension_by_flag: imports are added
 * automatically unless the caller asked for explicit ones, in which case a
 * missing import is an error instead of a silent addition.
 *
 * note the underlying API's parameter order:
 * aclspv_chck_extinst(name, ctx) vs aclspv_add_extinst_from_ctx(ctx, name).
 * */
ae2f_inline_f static aclspv_extinst_t aclutil_add_extinst_by_flag(
		x_aclspv_ctx* const ae2f_restrict	h_ctx,
		const char* ae2f_restrict const		c_name) {
	aclspv_extinst_t ID = aclspv_chck_extinst(c_name, h_ctx);

	if(ID) return ID;

	ae2f_ifnezerr(h_ctx->m_usrcfg.m_flag0
			& ACLSPV_CTX_FLAG0_EXPLICIT_EXTINST_IMPORT) {
		h_ctx->m_err = ACLSPV_COMPILE_NOIMPL_EXTINST;
		return 0;
	}

	return aclspv_add_extinst_from_ctx(h_ctx, c_name);
}

#define	aclutil_glsl_extinst(h)		aclutil_add_extinst_by_flag(h, "GLSL.std.450")

#endif
