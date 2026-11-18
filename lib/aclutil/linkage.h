#ifndef	aclutil_linkage_h
#define	aclutil_linkage_h

#include <aclspv.h>

#include <aclutil/wrdemit.h>

#include <string.h>

/**
 * emit `OpDecorate %c_id LinkageAttributes "<c_name>" <c_type>`.
 *
 * c_type is SpvLinkageTypeExport or SpvLinkageTypeImport. the word count is
 * computed from the name length because the opcode word carries it.
 *
 * @returns the new word count, 0 on failure.
 * */
ae2f_inline_f static aclspv_sz_t aclutil_emit_linkage(
		x_aclspv_vec* ae2f_restrict const	h_decorate,
		aclspv_sz_t				c_wrdcount,
		aclspv_id_t				c_id,
		const char* ae2f_restrict const		c_name,
		aclspv_wrd_t				c_type
		) {
	const size_t		len = (c_name ? strlen(c_name) : 0) + 1;	/** null included */
	const aclspv_wrd_t	pad = (aclspv_wrd_t)((len + 3u) >> 2u);
	aclspv_sz_t		c;

	ae2f_ifezerr(c = emit_opcode(
				h_decorate
				, c_wrdcount
				, SpvOpDecorate
				, 2u + pad + 1u
				))
		return 0;

	ae2f_ifezerr(c = aclutil_emit_wrd(h_decorate, c, c_id))
		return 0;

	ae2f_ifezerr(c = aclutil_emit_wrd(
				h_decorate
				, c
				, (aclspv_wrd_t)SpvDecorationLinkageAttributes))
		return 0;

	ae2f_ifezerr(c = aclutil_emit_str(
				h_decorate
				, c
				, c_name ? c_name : ""))
		return 0;

	ae2f_ifezerr(c = aclutil_emit_wrd(h_decorate, c, c_type))
		return 0;

	return c;
}

#endif
