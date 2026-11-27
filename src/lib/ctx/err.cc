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

#ifndef _aclspv_src_lib_ctx_err_cc
#define _aclspv_src_lib_ctx_err_cc

#include <aclspv.h>
#include <assert.h>
#include <ae2f/pat/macrofunc.h>

#include <util/log.h>
#include <ae2f/cc/unless.h>
#include <ae2f/cc/branches/naive.h>

/** prints an error when h->m_err has error */
ae2f_MAC() aclctx_err_log(
		const h_aclspv_ctx_t h
		) {
	ae2f_ifnezerr((h)->m_err) {
		if(sizeof(__FILE__) > sizeof(__ACLSPV_ROOT_PATH)) {
			err_call(fprintf, (stderr, "[errlog] %lx\t:: %s:%d\t\n"
						, (unsigned long)(h)->m_err
						, ((const char*)__FILE__ + sizeof(__ACLSPV_ROOT_PATH))
						, (int)__LINE__
					  ));
		}
	}
}

#if	!_ae2f_MACRO_GENERATED
#define	_aclctx_err_log	aclctx_err_log
#endif

ae2f_MAC() aclctx_err(
		const h_aclspv_ctx_t h
		, const aclspv_compile_t c
		) {
	ae2fcc_unless((h)->m_err) {
		(h)->m_err = (c);
	}

	_aclctx_err_log(h);
	/** it had assert(0); before, now it does not have one. */
}

#endif
