#ifndef	aclutil_mkname_c
#define aclutil_mkname_c

#include <.aclspv/util/u32_to_hex8.h>

#include <ae2f/pat/macrofunc.h>
#include <ae2f/cgoto.h>

#include <util/wrd.h>
#include <util/emitx.h>
#include <string.h>

#include <.aclspv/vec.h>

/** @brief it produces error on h_ctx */
ae2f_MAC((L, )) aclutil_mkname_dbg(const aclspv_id_t c_id, const char* const c_name, x_aclspv_ctx& cr_ctx, x_aclspv_vec& h_vec) {
	if((cr_ctx).m_flag0 & ACLSPV_CTX_FLAG0_SYMBOL) {
		const size_t L$$STRLEN_ORIG = strlen(c_name);	/** string len */
		const aclspv_wrdcount_t L$$STRLEN = (L$$STRLEN_ORIG & 0xFFFFFFFFu);
		const aclspv_wrd_t L$$POS = (cr_ctx).m_count.m_name;

		ae2f_ifezerr((size_t)L$$STRLEN == L$$STRLEN_ORIG) {
			(cr_ctx).m_err = ACLSPV_COMPILE_TOO_BIG;
			goto L$$LBL_DONE;
		}

		_aclspv_grow_vec(_aclspv_malloc, _aclspv_free
				, h_vec, strlen(c_name) + 10
				);

		ae2f_expected_but_else((h_vec).m_p) {
			goto L$$LBL_FAIL;
		}

		ae2f_reinterpret_cast(char* ae2f_restrict, (h_vec).m_p)[8] = ':';
		ae2f_reinterpret_cast(char* ae2f_restrict, (h_vec).m_p)[9] = '\0';

		_util_u32_to_hex8((c_id), (ae2f_reinterpret_cast(char* ae2f_restrict, (h_vec).m_p)));
		strcat(ae2f_reinterpret_cast(char* ae2f_restrict, (h_vec).m_p), (c_name));

		{
			int CND =
				ae2f_expected_not_strict(!
						(((cr_ctx).m_count.m_name = aclutil_emitx_2(
											 &(cr_ctx).m_section.m_name
											 , (cr_ctx).m_count.m_name
											 , SpvOpName
											 , (c_id)
											))
						))
				|| ae2f_expected_not_strict(!
						((cr_ctx).m_count.m_name = util_emit_str(
							&(cr_ctx).m_section.m_name	
							, (cr_ctx).m_count.m_name
							, ae2f_reinterpret_cast(const char* ae2f_restrict const, (h_vec).m_p)
							))
						);

			ae2f_ifnezerr(CND) goto L$$LBL_FAIL;
		}

		ae2fpat_assertassume(get_wrd_of_vec(&(cr_ctx).m_section.m_name));

		set_oprnd_count_for_opcode(
				get_wrd_of_vec(
					&(cr_ctx).m_section.m_name)[L$$POS]
				, ((cr_ctx).m_count.m_name - L$$POS - 1));
		goto L$$LBL_DONE;

		ae2f_unreachable();
		L$$LBL_FAIL:
			(cr_ctx).m_err = ACLSPV_COMPILE_ALLOC_FAILED;

	}

	L$$LBL_DONE: (void)0;
}

#if	!__ae2f_MACRO_GENERATED
#define	_aclutil_mkname_dbg(L, c_id, c_name, cr_ctx, h_vec)	aclutil_mkname_dbg(c_id, c_name, cr_ctx, h_vec)
#endif

ae2f_inline ae2f_unused static void aclutil_mkname_dbg_fn(
		const aclspv_id_t c_id,
		const char* const c_name,
		x_aclspv_ctx* ae2f_restrict cr_ctx,
		x_aclspv_vec* ae2f_restrict h_vec
		) {
	ae2fpat_assertassume(c_id);
	ae2fpat_assertassume(c_name);
	ae2fpat_assertassume(cr_ctx);
	ae2fpat_assertassume(h_vec);
	_aclutil_mkname_dbg(L, c_id, c_name, *cr_ctx, *h_vec);
}

#endif
