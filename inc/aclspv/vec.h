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

/** this is not to be necessarily public. */
#ifndef	aclspv_vec_h
#define aclspv_vec_h

#include "./ctx/common.h"
#include "./wrd.h"

#include <stddef.h>

#if	0
#include <.ae2f/pat/vec.h>
#define	aclspv_vec(T) struct			\
{						\
	h_aclspv_ctx_t m_ctx;			\
	void*	ae2f_restrict m_base;		\
	T*	ae2f_restrict m_vec;		\
	size_t m_sz; aclspv_wrd_t m_nel;	\
}
#endif

typedef struct {
	/** should be used */
	void* ae2f_restrict 	m_p;

	/** should be freed */
	void* ae2f_restrict 	m_base;
	h_aclspv_ctx_t		m_ctx;

	aclspv_wrd_t		m_sz, m_sz_real;
} x_aclspv_vec;

#endif
