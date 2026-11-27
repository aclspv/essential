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

#include "../fn.h"
#include "../implutil.h"
#include <aclmem.h>
#include <aclty.h>

aclty_t* aclty_cfg_fn(
		aclspv_id_t	c_ret_id,
		h_aclspv_ctx_t	h_ctx
		) {
	struct __ret r;
	aclty_t t;

	aclmemzero(&t, sizeof(t));
	t.m_class = ACLTY_FN;
	t.m_info.m_fn.m_ret_id = c_ret_id;
	t.m_field0 |= ACLTY_FIELD0_NOPOD;

	r = s_ret_new(h_ctx, &t, 0, 1);
	ae2f_ifezerr(r.m_id)
		return 0;

	return aclty_find(r.m_id, h_ctx);
}
