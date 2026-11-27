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

#ifndef	aclctx_err_h
#define aclctx_err_h

#include <aclspv.h>
#include <util/log.h>

#include <.aclspv/ctx/err.h>

/** error state check */
#define	aclctx_err_check(h)	_aclctx_err_log(h); ae2f_ifnezerr((h)->m_err)
#define	aclctx_err_check2(h, c)	_aclctx_err(h, c); ae2f_ifnezerr((h)->m_err)

#endif
