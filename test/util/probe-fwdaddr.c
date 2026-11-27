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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <acltest/onefile.h>
#include <acltest/compile.h>

static const char* const L[] = {
"int a(int*);\n",
"int b(int* v) {\n",
"    return a(v);\n",
"}\n",
"__attribute__((annotate(\"aclspv_execmodel(5)\")))\n",
"void main(void) {\n",
"    int aa = 0;\n",
"    int pa = b(&aa);\n",
"    aa = aa + 34;\n",
"}\n",
"int a(int* p) {\n",
"    return *p + '0';\n",
"}\n",
};

static char s_src[1024];

int main(void) {
	struct CXUnsavedFile f[2];
	const char* A[] = {"main.c"};
	aclspv_wrd_t* spv = 0;
	aclspv_wrdcount_t n = 0;
	aclspv_compile_t R;
	FILE* fp;

	acltestcfg();
	acltest_concat(s_src, L, (unsigned)(sizeof(L)/sizeof(L[0])));

	f[0].Contents = s_src; f[0].Filename = "main.c"; f[0].Length = (unsigned long)strlen(s_src);
	f[1].Contents = s_oclbase; f[1].Filename = "opencl-c-base.h"; f[1].Length = sizeof(s_oclbase)-1;

	R = dbg_aclspv_compile(f, 2, A, 1, &n, &spv, 0, ACLSPV_CTX_FLAG0_SYMBOL);
	printf("ret=%d n=%u\n", (int)R, (unsigned)n);
	if(!R && spv) {
		fp = fopen("/tmp/opencode/fwdaddr.spv", "wb");
		if(fp) { fwrite(spv, sizeof(aclspv_wrd_t), (size_t)n, fp); fclose(fp); }
	}
	free(spv);
	return 0;
}
