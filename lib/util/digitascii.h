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

#ifndef	aclutil_digitascii_h
#define aclutil_digitascii_h

#define		aclutil_ascii_to_uint(a)	\
	((a) == '1' ? 0x1 :			\
	(a) == '2' ? 0x2 :			\
	(a) == '3' ? 0x3 :			\
	(a) == '4' ? 0x4 :			\
	(a) == '5' ? 0x5 :			\
	(a) == '6' ? 0x6 :			\
	(a) == '7' ? 0x7 :			\
	(a) == '8' ? 0x8 :			\
	(a) == '9' ? 0x9 : 0x0)

#define		aclutil_is_digit(a)	\
	((a) == '0' || (a) == '1' || (a) == '2' || (a) == '3'	\
	(a) == '4' || (a) == '5' || (a) == '6' || (a) == '7'	\
	(a) == '8' || (a) == '9')


#endif
