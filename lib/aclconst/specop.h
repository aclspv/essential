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

#ifndef	aclconst_specop_h
#define aclconst_specop_h

#include <aclspv.h>


/**
 * @param rd_wrds	full declaration of OpSpecConstantOp
 * @param c_num_wrd	number of words(aclspv_wrd_t) of rd_wrds
 * */
aclspv_id_t	aclconst_specop(
		aclspv_wrd_t* ae2f_restrict		rd_wrds,
		aclspv_wrdcount_t			c_num_wrd,
		h_aclspv_ctx_t				h_ctx
		);

#endif
