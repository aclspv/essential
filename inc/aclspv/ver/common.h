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

#ifndef	aclspv_ver_common_h
#define aclspv_ver_common_h

#include <ae2f/Cast.h>

/** ((c & c_and) << c_pad) */
#define aclspv_and_pad(c, c_and, c_pad)	\
	((ae2f_static_cast(aclspv_wrd_t, (c)) & ae2f_static_cast(aclspv_wrd_t, (c_and))) \
	 << ae2f_static_cast(aclspv_wrd_t, (c_pad)))

#endif
