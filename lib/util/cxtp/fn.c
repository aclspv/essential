#include "./common.h"

#include <util/iddef.h>
#include <util/emitx.h>
#include <ae2f/cc/memset.h>

aclspv_id_t aclutil_mk_cxtp_fn(
		const aclspv_id_t	c_ret_id,
		const h_aclspv_ctx_t	h_ctx
		)
{
	aclutil_cxtp	TAR;
	ae2fcc_memset(&TAR, 0, sizeof(TAR));
	TAR.m_class				= ACLUTIL_CXTYPE_FUNC_NOPRM;
	TAR.m_info.m_fn_no_proto.m_ret_id	= c_ret_id;

	(void)(aclid_mk_default_id(c_ret_id, h_ctx));

	{
		const struct __ret ID = s_ret(h_ctx, &TAR);
		ae2f_expected_but_else(ID.m_id) return 0;
		unless(ID.m_isfirst)		return ID.m_id;

		ae2f_expected_but_else(h_ctx->m_count.m_types = util_emitx_3(
					&h_ctx->m_section.m_types
					, h_ctx->m_count.m_types
					, SpvOpTypeFunction
					, ID.m_id
					, c_ret_id
					)) return 0;

		return ID.m_id;
	}
}

aclspv_id_t aclutil_mk_cxtp_fn1(
		const CXCursor		c_cursor_compound,
		const CXType		c_type_compound,
		const h_aclspv_ctx_t	h_ctx
		)
{
	aclutil_cxtp	TAR;
	ae2fcc_memset(&TAR, 0, sizeof(TAR));
	TAR.m_class				= ACLUTIL_CXTYPE_FUNC;
	TAR.m_info.m_fn.m_type			= c_type_compound;
	TAR.m_info.m_fn.m_cursor		= c_cursor_compound;

	{
		const struct __ret ID = s_ret(h_ctx, &TAR);
		ae2f_expected_but_else(ID.m_id) return 0;
		unless(ID.m_isfirst)		return ID.m_id;

		{
			int ARGC			= clang_getNumArgTypes(c_type_compound);
			aclspv_wrd_t	ARGC_IDX	= (aclspv_wrd_t)ARGC;
			const aclspv_wrd_t	ANCHOUR	= h_ctx->m_count.m_types;
			aclspv_wrd_t* WORDS;

			ae2f_unexpected_but_if(ARGC <= 0) {
				const CXType RET	= clang_getResultType(c_type_compound);
				const aclspv_id_t RETID	= aclutil_mk_cxtp_by_cursor(
						c_cursor_compound
						,RET 
						, h_ctx);

				ae2f_expected_but_else(RETID) return 0;
				return aclutil_mk_cxtp_fn(RETID, h_ctx);
			}	/** fallback: argc was under 0 */

			ae2f_expected_but_else(h_ctx->m_count.m_types = util_emit_wrd(
						&h_ctx->m_section.m_types
						, h_ctx->m_count.m_types + (aclspv_wrd_t)ARGC + 2
						, 0)) return 0;


			WORDS = ((aclspv_wrd_t* ae2f_restrict)h_ctx->m_section.m_types.m_p);
			{ ae2f_assume(WORDS); }

			WORDS[ANCHOUR] = SpvOpTypeFunction;
			WORDS[ANCHOUR + 1] = ID.m_id;
			{
				const CXType RET	= clang_getResultType(c_type_compound);
				const aclspv_id_t RETID	= aclutil_mk_cxtp_by_cursor(
						c_cursor_compound
						,RET 
						, h_ctx);

				ae2f_expected_but_else(RETID) {
					h_ctx->m_err = ACLSPV_COMPILE_CXTP_ZERO;
					return 0;
				}
				WORDS[ANCHOUR + 2] = RETID;
			}

			while(ARGC_IDX--) {
				const CXType ARG		= clang_getArgType(
						c_type_compound
						, ARGC_IDX
						);
				const aclspv_wrd_t ARG_ID	= aclutil_mk_cxtp_by_cursor(
						c_cursor_compound
						, ARG
						, h_ctx
						);

				ae2f_expected_but_else(ARG_ID)	{
					h_ctx->m_err = ACLSPV_COMPILE_CXTP_ZERO;
					return 0;
				}

				WORDS[ARGC_IDX + ANCHOUR + 3] = ARG_ID;
			}
		}

		return ID.m_id;
	}
}
