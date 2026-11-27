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

#include <aclspv.h>

#include "../../lib/aclcur/lbl/pure.h"
#include "../../lib/aclsym/entp.h"
#include "../../lib/aclsym/fn.h"

#include <acltest/onefile.h>

#include <stdio.h>
#include <string.h>

/** compile one entry point and check the function-level purity bits the
 * pipeline captured. the cursor arena is reset at FunctionEnd, so the
 * durable answer is the pass-1 function symbol; the aclcur predicates are
 * only valid inside that carve/emit window (see aclcur_fn_cfg_pure). */
static int s_check(
		const char*	c_src,
		aclspv_bool_t	c_expect_const,
		aclspv_bool_t	c_expect_pure
		) {
	x_aclspv_ctx		ctx;
	aclspv_usrcfg_t		cfg;
	struct CXUnsavedFile	files[2];
	const char*		ARG[] = {"main.c"};
	aclspv_compile_t	ret;
	aclsym_entp_t*		p_entp;
	aclspv_wrd_t		c_mask;
	aclspv_wrd_t		c_want;

	memset(&cfg, 0, sizeof(cfg));

	files[0].Contents	= c_src;
	files[0].Filename	= "main.c";
	files[0].Length		= (unsigned long)strlen(c_src);

	files[1].Contents	= s_oclbase;
	files[1].Filename	= "opencl-c-base.h";
	files[1].Length		= sizeof(s_oclbase) - 1;

	if(aclspv_init_ctx(&ctx, &cfg)) {
		fputs("[fnpure] init failed\n", stderr);
		return 1;
	}

	ret = aclspv_ctx_build3(&ctx, files, 2, ARG, 1, 0);
	if(ret != ACLSPV_COMPILE_OK) {
		fprintf(stderr, "[fnpure] compile failed: %d\n", (int)ret);
		aclspv_fini_ctx(&ctx);
		return 1;
	}

	/** an entry point is not a call target: one entry record, and the one
	 * symbol the gather pass keeps for it (ENTRY — never NAMED) so a build
	 * without a source can tell whether its body went out. */
	if(ctx.m_fnlist.m_num_entp != 1 || !ctx.m_ctx_slave
			|| ctx.m_ctx_slave->m_fnlist.m_num_fn != 1) {
		fprintf(stderr, "[fnpure] function count: %u entry, %u symbol\n"
				, (unsigned)ctx.m_fnlist.m_num_entp
				, (unsigned)(ctx.m_ctx_slave
					? ctx.m_ctx_slave->m_fnlist.m_num_fn : 0));
		aclspv_fini_ctx(&ctx);
		return 1;
	}

	p_entp = &((aclsym_entp_t*)ctx.m_fnlist.m_entp.m_p)[0];

	c_mask = aclsym_fn_flag0(ACLSYM_FN_FLAG0_CONST)
		| aclsym_fn_flag0(ACLSYM_FN_FLAG0_PURE);
	c_want = (c_expect_const ? aclsym_fn_flag0(ACLSYM_FN_FLAG0_CONST) : 0)
		| (c_expect_pure ? aclsym_fn_flag0(ACLSYM_FN_FLAG0_PURE) : 0);

	if((p_entp->m_flag0 & c_mask) != c_want) {
		fprintf(stderr, "[fnpure] flags 0x%x, want 0x%x in 0x%x\n"
				, (unsigned)p_entp->m_flag0
				, (unsigned)c_want
				, (unsigned)c_mask);
		aclspv_fini_ctx(&ctx);
		return 1;
	}

	aclspv_fini_ctx(&ctx);
	return 0;
}

/** writes an Output: nothing of the buffer is read, so const; the write
 * escapes, so not pure. loading the Output pointer to form its address is
 * not a read of state. */
static const char s_src_out[] =
"typedef int i32;\n"
"__attribute__((annotate(\"aclspv_execmodel(5)\")))\n"
"__attribute__((annotate(\"aclspv_wrkgroup_size(1, 1, 1)\")))\n"
"void t_fn_out(\n"
"    __attribute__((annotate(\"aclspv_storage_class(3)\"))) i32* out\n"
") {\n"
"    *out = 7;\n"
"}\n";

/** reads a storage buffer into a Function-storage local: the read is
 * external, the local write is not. so not const, still pure. */
static const char s_src_in[] =
"typedef int i32;\n"
"__attribute__((annotate(\"aclspv_execmodel(5)\")))\n"
"__attribute__((annotate(\"aclspv_wrkgroup_size(1, 1, 1)\")))\n"
"void t_fn_in(\n"
"    __attribute__((annotate(\"aclspv_storage_class(12)\"))) i32* in\n"
") {\n"
"    i32 x = *in;\n"
"    x = x + 1;\n"
"}\n";

int main(void) {
	acltestcfg();

	if(s_check(s_src_out, 1, 0))
		return 1;

	if(s_check(s_src_in, 0, 1))
		return 1;

	return 0;
}
