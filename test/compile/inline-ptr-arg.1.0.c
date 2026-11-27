#include <aclspv.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <acltest/onefile.h>
#include <acltest/compile.h>
#include <acltest/spirv.h>

/**
 * a pointer argument binds to an inlined callee.
 *
 * a plain `i32*` parameter is a pointer to Function storage (aclty's default
 * for an unannotated pointer, see by_cursor/common.c), and the address of a
 * local is a pointer to Function storage too, so the two match and the call
 * can be carved into the caller.
 *
 * the argument that reaches the inliner must be the typed pointer VALUE: the
 * call path unwraps the address's OpCopyObject so an OpFunctionCall names the
 * memory object, and handing that unwrapped id to the inliner gave it a
 * storage id whose type is the pointee. the callee's own `*p` then failed to
 * carve (`ACLSPV_COMPILE_UNHANDLED_CARVE`) for a body that had nothing wrong
 * with it.
 * */
static const char* const s_src_lines[] = {
"typedef int i32;\n",
"__attribute__((always_inline)) i32 get(i32* p) { return *p; }\n",
"__attribute__((always_inline)) void vset(i32* p, i32 a) { *p = a; return; }\n",
"__attribute__((annotate(\"aclspv_execmodel(0)\")))\n",
"void entry(__attribute__((annotate(\"aclspv_storage_class(3)\"))) i32* out)\n",
"{ i32 t = 5; vset(&t, 7); *out = get(&t); }\n"
};

static char s_src[1024];

int main(void) {
	struct CXUnsavedFile files[2];
	const char* ARG[] = {"main.c"};
	aclspv_compile_t RET;
	aclspv_wrd_t* spirv = 0;
	aclspv_wrdcount_t count = 0;
	unsigned calls, vars;
	acltest_spirv_stats stats;

	acltestcfg();
	acltest_concat(s_src, s_src_lines,
			(unsigned)(sizeof(s_src_lines) / sizeof(s_src_lines[0])));

	files[0].Contents = s_src;
	files[0].Filename = "main.c";
	files[0].Length = (unsigned)strlen(s_src);

	files[1].Contents = s_oclbase;
	files[1].Filename = "opencl-c-base.h";
	files[1].Length = sizeof(s_oclbase) - 1;

	RET = dbg_aclspv_compile(files, 2, ARG, 1, &count, &spirv, 0, 0);

	printf("aclspv_compile: %d\n", (int)RET);
	printf("output spirv: %p, %u\n", (void*)spirv, count);

	if(RET != ACLSPV_COMPILE_OK || !spirv) {
		fprintf(stderr, "[inline-ptr-arg] compile failed: %d\n", (int)RET);
		free(spirv);
		return EXIT_FAILURE;
	}

	if(acltest_spirv_scan(spirv, count, &stats)) {
		fputs("[inline-ptr-arg] malformed spir-v instruction stream\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	calls = acltest_spirv_count_op(spirv, count,
			(unsigned)ACLTEST_SPV_OP_FUNCTION_CALL);
	vars = acltest_spirv_count_var_in(spirv, count,
			(aclspv_wrd_t)SpvStorageClassFunction);

	printf("calls=%u fn_vars=%u\n", calls, vars);

	/** both always_inline bodies are carved into the entry; the local `t`
	 * keeps its Function-storage OpVariable because its address escapes. */
	if(calls || !vars) {
		fputs("[inline-ptr-arg] the pointer-taking callees were not inlined\n",
				stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	if(!stats.m_targets_ok) {
		fputs("[inline-ptr-arg] a branch target names no label\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	{
		FILE* const fp = fopen(ACLTEST_OUT, "wb");

		if(!fp) {
			fprintf(stderr, "[inline-ptr-arg] cannot write %s\n", ACLTEST_OUT);
			free(spirv);
			return EXIT_FAILURE;
		}

		fwrite(spirv, sizeof(aclspv_wrd_t), (size_t)count, fp);
		fclose(fp);
	}

	free(spirv);
	return EXIT_SUCCESS;
}
