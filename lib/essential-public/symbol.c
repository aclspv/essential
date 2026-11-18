#include <aclspv.h>
#include <aclmem.h>
#include <aclctx/id.h>
#include <aclctx/err.h>
#include <ae2f/cc/pause.h>
#include <aclutil/emitx.h>

ACLSPV_ABI_LINK 
aclspv_id_t aclspv_cfg_symbol(
		const char* ae2f_restrict rd,
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

	/** one name, one OpName: a repeated name returns the id it already
	 * carries. the chunk is header, target id, then the zero-padded bytes,
	 * so the whole-word compare below is exact. */
	i = 0;
	while(i < lenchunk) {
		const wrd_t read_opn = (chunk[i] >> 16) & 0xFFFF;
		aclspv_bool_t c_same = 1;
		wrd_t k;

		/** a different chunk size is a different name. */
		if(opn != read_opn) { i += read_opn; continue; }

		for(k = 0; k < aligned && c_same; ++k) {
			const wrd_t n_base = k << 2;
			wrd_t c_pack = 0;
			wrd_t b;

			for(b = 0; b < 4; ++b) {
				const wrd_t n_idx = n_base + b;

				if(n_idx < c)
					c_pack |= ((wrd_t)(unsigned char)rd[n_idx])
						<< (b << 3);
			}

			if(c_pack != chunk[i + 2 + k])
				c_same = 0;
		}

		if(c_same)
			return chunk[i + 1];

		i += read_opn;
	}

	/** append: header, target id, then the zero-padded name. */
	ae2f_ifezerr(h->m_count.m_name_symbol = aclutil_emit_wrd(
				&h->m_section.m_name_symbol
				, lenchunk + opn - 1
				, 0
				)) return 0;

	chunk = h->m_section.m_name_symbol.m_p;

	{
		const wrd_t n_end = lenchunk + opn;
		wrd_t k;

		for(k = lenchunk; k < n_end; ++k)
			chunk[k] = 0;
	}

	chunk[lenchunk]		= (opn << 16) | (wrd_t)(SpvOpName);
	chunk[lenchunk + 1]	= c_new && c_new < h->m_id
		? c_new
		: aclctx_newid(h);
	aclmemcpy(&chunk[lenchunk + 2], rd, c);

	return chunk[lenchunk + 1];
}
