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

#include "../../lib/aclutil/scale.h"

#include <stdio.h>
#include <string.h>

#define	CHECK(c)	do { \
	if(!(c)) { \
		fprintf(stderr, "check failed: %s:%d\n", __FILE__, __LINE__); \
		return 1; \
	} \
} while(0)

int main(void) {
	x_aclspv_ctx	ctx;
	x_aclspv_vec	alloc;

	aclspv_usrcfg_t	usrcfg;
	memset(&usrcfg, 0, sizeof(usrcfg));

	/** the test targets are built with -DNDEBUG, so assert() is erased.
	 * every check is an explicit branch. */
	CHECK(!aclspv_init_ctx(&ctx, &usrcfg));
	aclmem_init(&alloc, &ctx);
	CHECK(init_scale(&alloc, 0));

	CHECK(mk_scale_from_vec(&alloc, 0));

#if	1
	CHECK(mk_scale_from_vec(&alloc, 0));
	CHECK(mk_scale_from_vec(&alloc, 0));
	CHECK(mk_scale_from_vec(&alloc, 0));
	CHECK(mk_scale_from_vec(&alloc, 0));
#endif

	CHECK(get_scale_from_vec(&alloc, 0));
	CHECK(get_scale_from_vec(&alloc, 1));
	CHECK(get_scale_from_vec(&alloc, 2));
	CHECK(get_scale_from_vec(&alloc, 3));

	aclmem_fini(&alloc);
	aclspv_fini_ctx(&ctx);
	return 0;
}
