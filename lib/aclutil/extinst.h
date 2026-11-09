#ifndef	aclutil_extinst_h
#define	aclutil_extinst_h

#include <aclspv.h>
#include <aclctx/id.h>
#include <aclutil/wrdemit.h>

#include <string.h>

/** id of an `OpExtInstImport` named `c_name`, emitting it on first use.
 * returns 0 on allocation failure. */
ae2f_inline_f static aclspv_id_t aclutil_extinst(
		h_aclspv_ctx_t	h_ctx,
		const char*	c_name
		) {
	const aclspv_wrd_t* ae2f_restrict p;
	const aclspv_wrd_t	LEN = (aclspv_wrd_t)(strlen(c_name) + 1);
	const aclspv_wrd_t	NAME_WRDS = (LEN + 3) >> 2;
	aclspv_wrd_t		i;
	aclspv_wrd_t		c;
	aclspv_id_t		id;

	ae2f_ifezerr(h_ctx)	return 0;
	ae2f_ifezerr(c_name)	return 0;

	/** the import belongs to the extensions section; reuse it if it is
	 * already there. */
	p = aclutil_get_wrd_of_vec(&h_ctx->m_section.m_ext);
	for(i = 0; i < h_ctx->m_count.m_ext; ) {
		const aclspv_wrd_t WC = p[i] >> 16;
		const aclspv_wrd_t OP = p[i] & 0xFFFF;

		if(!WC || i + WC > h_ctx->m_count.m_ext)
			break;

		if(OP == SpvOpExtInstImport && WC >= 2 + NAME_WRDS
				&& !memcmp(&p[i + 2], c_name, LEN)) {
			return p[i + 1];
		}

		i += WC;
	}

	/** [opcode|wc][result id][name words...] */
	id = aclctx_newid(h_ctx);
	ae2f_ifezerr(id) return 0;

	c = h_ctx->m_count.m_ext;
	c = aclutil_emit_wrd(
			&h_ctx->m_section.m_ext
			, c
			, aclspv_opcode_to_wrd(SpvOpExtInstImport, 1 + NAME_WRDS));
	ae2f_ifezerr(c) return 0;

	c = aclutil_emit_wrd(&h_ctx->m_section.m_ext, c, id);
	ae2f_ifezerr(c) return 0;

	c = aclutil_emit_str(&h_ctx->m_section.m_ext, c, c_name);
	ae2f_ifezerr(c) return 0;

	h_ctx->m_count.m_ext = c;
	return id;
}

#define	aclutil_glsl_extinst(h)		aclutil_extinst(h, "GLSL.std.450")

#endif
