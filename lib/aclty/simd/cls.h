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

#ifndef	aclty_simd_cls_h
#define aclty_simd_cls_h

#include <aclty/simd.h>
#include <aclty/sclr.h>

ae2f_inline_f static aclspv_bool_t aclty_is_simd_flt(aclspv_id_t c, h_aclspv_ctx_t h) {
	return aclty_is_sclr_flt(aclty_get_simd_sclr((c), (h)));
}

ae2f_inline_f static aclspv_bool_t aclty_is_simd_int(aclspv_id_t c, h_aclspv_ctx_t h) {
	return aclty_is_sclr_int(aclty_get_simd_sclr((c), (h)));
}

ae2f_inline_f static aclspv_bool_t aclty_is_simd_uint(aclspv_id_t c, h_aclspv_ctx_t h) {
	return aclty_is_sclr_uint(aclty_get_simd_sclr((c), (h)));
}

ae2f_inline_f static aclty_sclr_width_t aclty_get_simd_sclr_width(aclspv_id_t c, h_aclspv_ctx_t h) {
	return aclty_get_sclr_width(aclty_get_simd_sclr(c, h));
}

#endif
