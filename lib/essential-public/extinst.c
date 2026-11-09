#include <aclspv.h>


#include <string.h>
#include <aclutil/wrdemit.h>
#include <aclctx/id.h>

ACLSPV_ABI_IMPL ae2f_ccpure ae2f_noexcept aclspv_extinst_t aclspv_chck_extinst(
		const char* ae2f_restrict rd,
		h_aclspv_ctx_t h_ctx
		) { ae2f_ifezerr_strict(h_ctx && h_ctx->m_section.m_extinst.m_p && rd) return 0; {
	
#define	BUF	aclutil_get_wrd_of_vec(&h_ctx->m_section.m_extinst)
	aclspv_wrd_t		I	= 0;
	const aclspv_wrd_t	C	= h_ctx->m_count.m_extinst;
	const size_t		ELEN	= strlen(rd) + 1;
	const aclspv_wrd_t	WELEN	= ELEN > 0xFFFFFFFF ? 0 : (aclspv_wrd_t)ELEN; 
	const aclspv_wrd_t	WCEXT	= ((WELEN + 3) >> 2) + 2;

	ae2f_ifnezerr(h_ctx->m_err)	return 0;

	while(I < C) {
		const aclspv_wrd_t	SKIP	= (BUF[I] >> 16) & 0xffff;

		if(SKIP == WCEXT
				&&	!strcmp(rd, (const char* ae2f_restrict)&BUF[I + 2]))
			return BUF[I + 1];

		if(SKIP > WCEXT)	break;
		ae2f_ifezerr(SKIP)	break;

		I += SKIP;
	}

	return 0;
#undef	BUF
}}

ACLSPV_ABI_IMPL ae2f_noexcept aclspv_extinst_t aclspv_add_extinst_from_ctx(
		const h_aclspv_ctx_t ae2f_restrict	h_ctx,
		const char* ae2f_restrict const		rd
		) {
	aclspv_wrd_t		I	= 0;
	const aclspv_wrd_t	C	= h_ctx ? h_ctx->m_count.m_extinst : 0;
	const size_t		ELEN	= rd ? 1 + strlen(rd) : 0;
	const aclspv_wrd_t	WELEN	= ae2f_bll_sel_bool1(
			aclspv_wrd_t
			, (aclspv_wrd_t)(ELEN > (size_t)0xFFFFFFFF)
			, 0 
			, (aclspv_wrd_t)ELEN
			);
	const aclspv_wrd_t	WCEXT	= ((WELEN + 3) >> 2) + 2;

	ae2f_ifezerr_strict(h_ctx && rd) return 0;
	ae2f_ifnezerr(h_ctx->m_err)		return 0;
	unless(h_ctx->m_section.m_extinst.m_p) goto LBL_NOT_FOUND;

	{
#define	BUF	aclutil_get_wrd_of_vec(&h_ctx->m_section.m_extinst)

		/** entries are kept sorted by word count (ascending). */
		while(I < C) {
			const aclspv_wrd_t	SKIP	= (BUF[I] >> 16) & 0xffff;

			if(SKIP == WCEXT
					&&	!strcmp(rd, (const char* ae2f_restrict)&BUF[I + 2]))
				return BUF[I + 1];

			if(SKIP > WCEXT)	break;
			ae2f_ifezerr(SKIP)	break;

			I += SKIP;
		}
#undef	BUF
	}

LBL_NOT_FOUND:
	dbg_puts(rd);
	dbg_puts("extinst::edge");
	unless(I < C) {
		const aclspv_extinst_t ID = aclctx_newid(h_ctx);

		ae2f_ifezerr(h_ctx->m_count.m_extinst = aclutil_emit_wrd(
					&h_ctx->m_section.m_extinst
					, h_ctx->m_count.m_extinst
					, SpvOpExtInstImport | (WCEXT << 16))) {
			return 0;
		}

		ae2f_ifezerr(h_ctx->m_count.m_extinst = aclutil_emit_wrd(
					&h_ctx->m_section.m_extinst
					, h_ctx->m_count.m_extinst
					, ID)) {
			return 0;
		}

		ae2f_ifezerr(h_ctx->m_count.m_extinst = aclutil_emit_str(
					&h_ctx->m_section.m_extinst
					, h_ctx->m_count.m_extinst
					, rd)) {
			return 0;
		}

		return ID;
	}

	dbg_puts(rd);
	dbg_puts("extinst::mid");

	ae2f_ifezerr(h_ctx->m_count.m_extinst = aclutil_emit_wrd(
				&h_ctx->m_section.m_extinst
				, h_ctx->m_count.m_extinst + WCEXT - 1
				, 0)) {
		return 0;
	}

	_aclutil_vcopr_v(L, =, --
			, aclutil_get_wrd_of_vec(&h_ctx->m_section.m_extinst) + I + WCEXT - 1	+ (C - I)
			, aclutil_get_wrd_of_vec(&h_ctx->m_section.m_extinst) + I - 1 		+ (C - I)
			, C - I);

	aclutil_get_wrd_of_vec(&h_ctx->m_section.m_extinst)[I] = SpvOpExtInstImport | (WCEXT << 16);
	strcpy((char* ae2f_restrict)&aclutil_get_wrd_of_vec(&h_ctx->m_section.m_extinst)[I + 2], rd);
	return aclutil_get_wrd_of_vec(&h_ctx->m_section.m_extinst)[I + 1] = aclctx_newid(h_ctx);
}
