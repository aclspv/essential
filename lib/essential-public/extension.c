#include <aclspv.h>
#include <ae2f/bll.h>
#include <ae2f/cgoto.h>
#include <ae2f/cc/switchctl.h>
#include <string.h>
#include <aclutil/emitx.h>
#include <.aclspv/util/vcopr.h>
#include <util/log.h>

ACLSPV_ABI_IMPL ae2f_ccpure ae2f_noexcept aclspv_bool_t aclspv_chck_extension_from_ctx(
		const x_aclspv_ctx* const ae2f_restrict	rd_ctx,
		const char* ae2f_restrict const		c_ext
		) { ae2f_ifezerr_strict(rd_ctx && rd_ctx->m_section.m_ext.m_p && c_ext) return 0; {
	
#define	BUF	aclutil_get_wrd_of_vec(&rd_ctx->m_section.m_ext)
	aclspv_wrd_t		I	= 0;
	const aclspv_wrd_t	C	= rd_ctx->m_count.m_ext;
	/** null terminator included, same as `aclspv_add_extension_from_ctx` */
	const size_t		ELEN	= strlen(c_ext) + 1;
	const aclspv_wrd_t	WELEN	= ae2f_bll_sel_bool1(aclspv_wrd_t
			, (aclspv_wrd_t)(ELEN > (size_t)0xFFFFFFFF)
			, 0 
			, (aclspv_wrd_t)ELEN
			);
	const aclspv_wrd_t	WCEXT	= ((WELEN + 3) >> 2) + 1;

	ae2f_ifnezerr(rd_ctx->m_err)	return 0;

	while(I < C) {
		const aclspv_wrd_t	SKIP	= (BUF[I] >> 16) & 0xffff;

		if(SKIP == WCEXT
				&&	!strcmp(c_ext, (const char* ae2f_restrict)&BUF[I + 1]))
			return 1;

		if(SKIP > WCEXT)	break;
		ae2f_ifezerr(SKIP)	break;

		I += SKIP;
	}

	return 0;
#undef	BUF

	ae2f_unreachable();
} ae2f_unreachable(); }

ACLSPV_ABI_IMPL ae2f_ccpure ae2f_noexcept aclspv_bool_t aclspv_add_extension_from_ctx(
		x_aclspv_ctx* const ae2f_restrict	h_ctx,
		const char* ae2f_restrict const		c_ext
		) { 
	aclspv_wrd_t		I	= 0;
	const aclspv_wrd_t	C	= h_ctx ? h_ctx->m_count.m_ext : 0;
	const size_t		ELEN	= c_ext ? 1 + strlen(c_ext) : 0;
	const aclspv_wrd_t	WELEN	= ae2f_bll_sel_bool1(
			aclspv_wrd_t
			, (aclspv_wrd_t)(ELEN > (size_t)0xFFFFFFFF)
			, 0 
			, (aclspv_wrd_t)ELEN
			);
	const aclspv_wrd_t	WCEXT	= ((WELEN + 3) >> 2) + 1;

	ae2f_ifezerr_strict(h_ctx && c_ext) return 0;
	ae2f_ifnezerr(h_ctx->m_err)		return 0;
	unless(h_ctx->m_section.m_ext.m_p) goto LBL_NOT_FOUND;

	{
#define	BUF	aclutil_get_wrd_of_vec(&h_ctx->m_section.m_ext)

		/** entries are kept sorted by word count (ascending). */
		while(I < C) {
			const aclspv_wrd_t	SKIP	= (BUF[I] >> 16) & 0xffff;

			if(SKIP == WCEXT
					&&	!strcmp(c_ext, (const char* ae2f_restrict)&BUF[I + 1]))
				return 1;

			if(SKIP > WCEXT)	break;
			ae2f_ifezerr(SKIP)	break;

			I += SKIP;
		}
#undef	BUF
	}

LBL_NOT_FOUND:
	dbg_puts(c_ext);
	dbg_puts("extension::edge");
	unless(I < C) {
		ae2f_ifezerr(h_ctx->m_count.m_ext = aclutil_emit_wrd(
					&h_ctx->m_section.m_ext
					, h_ctx->m_count.m_ext
					, SpvOpExtension | (WCEXT << 16))) {
			return 0;
		}

		ae2f_ifezerr(h_ctx->m_count.m_ext = aclutil_emit_str(
					&h_ctx->m_section.m_ext
					, h_ctx->m_count.m_ext
					, c_ext)) {
			return 0;
		}
		return 1;
	}

	dbg_puts(c_ext);
	dbg_puts("extension::mid");

	ae2f_ifezerr(h_ctx->m_count.m_ext = aclutil_emit_wrd(
				&h_ctx->m_section.m_ext
				, h_ctx->m_count.m_ext + WCEXT - 1
				, 0)) {
		return 0;
	}

	_aclutil_vcopr_v(L, =, --
			, aclutil_get_wrd_of_vec(&h_ctx->m_section.m_ext) + I + WCEXT - 1	+ (C - I)
			, aclutil_get_wrd_of_vec(&h_ctx->m_section.m_ext) + I - 1 		+ (C - I)
			, C - I);

	aclutil_get_wrd_of_vec(&h_ctx->m_section.m_ext)[I] = SpvOpExtension | (WCEXT << 16);
	strcpy((char* ae2f_restrict)&aclutil_get_wrd_of_vec(&h_ctx->m_section.m_ext)[I + 1], c_ext);

	return 1;
}
