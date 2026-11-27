/*
 * Copyright 2026 dalmurii
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * See the LICENSE file for the full text, including LLVM Exceptions.
 */

#ifndef	aclty_by_cursor_sclr_h
#define aclty_by_cursor_sclr_h

#include <aclty.h>

typedef CXClientData cxclientdata_aclty_id_t;

static enum CXChildVisitResult s_sclr_flt_alias(CXCursor c, ae2f_unused CXCursor c_par, cxclientdata_aclty_id_t ae2f_restrict r) {
#if	__ACLSPV_TYPE_FLOAT_ALIAS
	aclty_id_t* ae2f_restrict R = r;
	if(*R) return CXChildVisit_Break;
	unless(c.kind == CXCursor_AnnotateAttr)
		return CXChildVisit_Continue;

	{
		const char* NDL;
		CXString TXT = clang_getCursorSpelling(c);
#define	ret(c)	{ *R = (c); clang_disposeString(TXT); return CXChildVisit_Break; }

#if	__ACLSPV_SCLR_BF16
		if((NDL = strstr(TXT.data, "__bf16")))
			ret(ACLSPV_ID_BF16);
		if((NDL = strstr(TXT.data, "__nv_bfloat16")))
			ret(ACLSPV_ID_BF16);
#endif

#if	__ACLSPV_SCLR_F8_E4M3
		if((NDL = strstr(TXT.data, "__f8_e4m3")))
			ret(ACLSPV_ID_F8_E4M3);

		if((NDL = strstr(TXT.data, "__fp8_e4m3")))
			ret(ACLSPV_ID_F8_E4M3);

		if((NDL = strstr(TXT.data, "__nv_fp8_e4m3")))
			ret(ACLSPV_ID_F8_E4M3);
#endif

#if	__ACLSPV_SCLR_F8_E5M2
		if((NDL = strstr(TXT.data, "__f8_e5m2")))
			ret(ACLSPV_ID_F8_E5M2);

		if((NDL = strstr(TXT.data, "__fp8_e5m2")))
			ret(ACLSPV_ID_F8_E5M2);

		if((NDL = strstr(TXT.data, "__nv_fp8_e5m2")))
			ret(ACLSPV_ID_F8_E5M2);
#endif

		clang_disposeString(TXT);
	}

	return CXChildVisit_Continue;
#else
	return CXChildVisit_Break;
#endif
}



static ae2f_ccconst aclty_id_t s_sclr(const CXType c_ty, ae2f_unused const CXCursor c_cur) {
	switch((aclspv_cxtykind_t)c_ty.kind) {
#define	VECCASE(c_case, c_ret)	\
		case c_case: return c_ret

		case CXType_SChar:
		VECCASE(CXType_Char_S		, ACLSPV_ID_I8);
		VECCASE(CXType_Bool		, ACLSPV_ID_BOOL);

		case CXType_UChar:
		VECCASE(CXType_Char_U		, ACLSPV_ID_U8);

		case CXType_Char16:
		VECCASE(CXType_Short		, ACLSPV_ID_I16);
		VECCASE(CXType_UShort		, ACLSPV_ID_U16);

		case CXType_Char32:
		VECCASE(CXType_Int		, ACLSPV_ID_I32);
		VECCASE(CXType_UInt		, ACLSPV_ID_U32);

		case CXType_LongLong:
		VECCASE(CXType_Long		, ACLSPV_ID_I64);

		case CXType_ULongLong:
		VECCASE(CXType_ULong		, ACLSPV_ID_U64);

#if	__ACLSPV_SCLR_BF16
		case CXType_BFloat16:
			return ACLSPV_ID_BF16;
#endif

#if	__ACLSPV_SCLR_F16
		VECCASE(CXType_Float16		, ACLSPV_ID_F16);
		VECCASE(CXType_Half		, ACLSPV_ID_F16);
#endif

		VECCASE(CXType_Float		, ACLSPV_ID_F32);
		VECCASE(CXType_Double		, ACLSPV_ID_F64);

#undef		VECCASE

		case CXType_Float128:
		{
			aclty_id_t r = 0;
			clang_visitChildren(c_cur, s_sclr_flt_alias, &r);
			return r;
		}

		default: return 0;
	}
}

#include <aclty/default.h>

ae2f_inline_f static aclty_id_t s_simd(
		const CXType		c_ty,
		const CXCursor		c_cur,
		const h_aclspv_ctx_t	h_ctx
		) {
	{
		const aclspv_id_t D = s_sclr(c_ty, c_cur);
		if(D) {
			return aclty_mk_default(D, h_ctx);
		}
	}

	dbg_prefix();
	dbg_call((void), fprintf(stderr, "cxtp_by_type2 tykind %u\n", (unsigned)c_ty.kind));

	switch((aclspv_cxtykind_t)c_ty.kind) {
		case CXType_Vector: case CXType_ExtVector:
			{
				const CXType ELM		= clang_getElementType(c_ty);
				const intmax_t _ELMCOUNT	= clang_getNumElements(c_ty);
				const aclspv_wrd_t ELMCOUNT	= 
							(aclspv_wrdmax_t)(_ELMCOUNT) & ~(aclspv_wrdmax_t)0xFFFFFFFF
							? (aclspv_wrd_t)1 
							: (aclspv_wrd_t)_ELMCOUNT
							;

				const aclspv_id_t	ELMID = s_sclr(ELM, c_cur);

				return aclty_mk_vec(
						ELMID
						, ELMCOUNT
						, h_ctx);
			}

		default:
			return 0;
	}

	ae2f_unreachable();
}

#endif
