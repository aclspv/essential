#include <aclspv.h>
#include <aclmem.h>
#include <aclctx/id.h>
#include <aclctx/err.h>
#include <ae2f/cc/pause.h>
#include <aclutil/emitx.h>

ACLSPV_ABI_IMPL 
aclspv_id_t aclspv_cfg_symbol(
		const char ae2fcc_arg_sarr_restrict(rd, ae2f_NONE),
		aclspv_wrd_t c,
		aclspv_id_t	c_new,
		h_aclspv_ctx_t h
		) {
	typedef aclspv_wrd_t wrd_t;
	const wrd_t aligned = (c >> 2) + 1;
	const wrd_t opn = 2 + aligned;
	const wrd_t lenchunk = h->m_count.m_name_symbol;
	wrd_t* ae2f_restrict chunk = h->m_section.m_name_symbol.m_p;
	wrd_t i;

	ae2f_ifnezerr_strict((!chunk) && (lenchunk)) {
		_aclctx_err(h, ACLSPV_COMPILE_UNHANDLED);
		return 0;
	}

#define	WRDSTR(rd, c_pad, c_idx)	(((rd)[(c_pad) + ((c_idx) >> 2)] >> (((c_idx) & 0x3) << 3)) & 0xFF)
#define STRWRDK(rd, c_idx, c_prv)	(((c_prv) ? (((wrd_t)(rd)[c_idx]) << ((c_idx) << 3)) : 0) | (wrd_t)(c_prv))
#define STRWRDK0(rd, c_idx)		((wrd_t)(rd)[c_idx])
#define	STRWRDK1(rd, c_idx)		STRWRDK(rd, c_idx + 1, STRWRDK0(rd, c_idx))
#define	STRWRDK2(rd, c_idx)		STRWRDK(rd, c_idx + 1, STRWRDK1(rd, c_idx))
#define	STRWRDK3(rd, c_idx)		STRWRDK(rd, c_idx + 1, STRWRDK2(rd, c_idx))
#define	STRWRDK4(rd, c_idx)		STRWRDK(rd, c_idx + 1, STRWRDK3(rd, c_idx))
#define STRWRD(rd, c_idx, c_len)	(c_idx < c_len ? STRWRDK4(rd, c_idx) : 0)


	i = 0; while(i < lenchunk) {
		const wrd_t read_opn = (chunk[i] >> 16) & 0xFFFF;
		wrd_t j;
		if(opn != read_opn) continue;	/** word count mismatch */
		for(j = opn; j --> 0 && STRWRD(rd, j, c) != chunk[i + 2]; ) ae2fcc_pause();
		if(j > opn) { puts("found"); return chunk[i + 1]; };

		i += read_opn;
	}

	ae2f_ifezerr(h->m_count.m_name_symbol = aclutil_emit_wrd(
				&h->m_section.m_name_symbol
				, lenchunk + opn - 1
				, 0
				)) return 0;

	chunk = h->m_section.m_name_symbol.m_p;
	chunk[lenchunk] = (opn << 16) | (wrd_t)(SpvOpName);
	aclmemcpy(&chunk[lenchunk + 2], rd, c);



	return (chunk[lenchunk + 1] = c_new && c_new < h->m_id ? c_new : aclctx_newid(h));
}
