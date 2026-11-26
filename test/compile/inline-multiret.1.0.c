#include <aclspv.h>
#include <stdio.h>
#include <stdlib.h>
#include <acltest/onefile.h>
#include <acltest/compile.h>
#include <acltest/spirv.h>

/**
 * multi-return always_inline bodies are inlined through the
 * single-iteration loop scaffold: every return becomes a branch to the
 * loop continue target and the result/done flag are carried by loop-header
 * phis. a return under a nested loop is not a structured exit and still
 * stays an OpFunctionCall. a return nested in a loop that is *mandatory*
 * (always_inline) is a rejection, not a silent fallback.
 *
 * also covers the inline policy: a plain `i32 f()` must still emit
 * `Inline` function control (only `noinline` opts out).
 * */
static const char* const s_src_lines[] = {
"typedef int i32;\n",
"__attribute__((always_inline)) i32 pick(i32 x)\n",
"{ if(x > 0) return 10; return 20; }\n",
"__attribute__((always_inline)) i32 both(i32 x)\n",
"{ if(x > 0) return 30; else return 40; }\n",
"__attribute__((always_inline)) i32 sw(i32 x)\n",
"{ switch(x){ case 1: return 50; default: return 60; } }\n",
"__attribute__((noinline)) i32 loopret(i32 x)\n",
"{ for(i32 i = 0; i < 3; i++){ if(i == x) return i; } return 70; }\n",
"__attribute__((noinline)) i32 ni(i32 x) { return x + 2; }\n",
"i32 plain(i32 x) { return x + 3; }\n",
"__attribute__((annotate(\"aclspv_execmodel(0)\")))\n",
"void entry(__attribute__((annotate(\"aclspv_storage_class(3)\"))) i32* out)\n",
"{ *out = pick(1) + both(2) + sw(3) + loopret(4) + ni(5) + plain(6); }\n"
};

static char s_src[1024];

/** walk OpFunction control words: [op][ret ty][id][control][fn ty]. */
static int s_count_fnctrl(const aclspv_wrd_t* rd, aclspv_wrdcount_t c) {
	aclspv_wrdcount_t i = 5;
	unsigned n_inline = 0, n_dont = 0, n_none = 0;

	while(i < c) {
		const unsigned wc = (unsigned)(rd[i] >> 16);
		const unsigned op = (unsigned)(rd[i] & 0xFFFFu);

		if(!wc || i + wc > c)
			return 1;

		if(op == (unsigned)ACLTEST_SPV_OP_FUNCTION && wc >= 5) {
			const aclspv_wrd_t ctrl = rd[i + 3];

			if(ctrl & (aclspv_wrd_t)ACLTEST_SPV_FNCTRL_INLINE)
				++n_inline;
			else if(ctrl & (aclspv_wrd_t)ACLTEST_SPV_FNCTRL_DONT_INLINE)
				++n_dont;
			else
				++n_none;
		}

		i += wc;
	}

	printf("fnctrl: inline=%u dontinline=%u none=%u\n", n_inline, n_dont, n_none);

	/** pick/both/sw/loopret/plain say Inline, ni says DontInline (the one
	 * opting out), and the entry point is not a call target: no policy. */
	return (n_inline == 4 && n_dont == 2 && n_none == 1) ? 0 : 1;
}

int main(void) {
	struct CXUnsavedFile files[2];
	const char* ARG[] = {"main.c"};
	aclspv_compile_t RET;
	aclspv_wrd_t* spirv = 0;
	aclspv_wrdcount_t count = 0;
	unsigned calls;
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
		fprintf(stderr, "[inline-multiret] compile failed: %d\n", (int)RET);
		free(spirv);
		return EXIT_FAILURE;
	}

	if(acltest_spirv_scan(spirv, count, &stats)) {
		fputs("[inline-multiret] malformed spir-v instruction stream\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	calls = acltest_spirv_count_op(spirv, count, (unsigned)ACLTEST_SPV_OP_FUNCTION_CALL);

	/** loopret (noinline, its body is not carvable), ni and plain remain
	 * calls; the three always_inline bodies are carved into the entry. */
	if(calls != 3) {
		fprintf(stderr, "[inline-multiret] unexpected OpFunctionCall count: %u\n", calls);
		free(spirv);
		return EXIT_FAILURE;
	}

	/** the control mask: `noinline` opts out, every other callable
	 * function (always_inline, and the plain one nobody asked about)
	 * says Inline, and the entry point carries none. */
	if(s_count_fnctrl(spirv, count) != 0) {
		fputs("[inline-multiret] unexpected OpFunction control\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	{
		FILE* const fp = fopen(ACLTEST_OUT, "wb");

		if(!fp) {
			fprintf(stderr, "[inline-multiret] cannot write %s\n", ACLTEST_OUT);
			free(spirv);
			return EXIT_FAILURE;
		}

		fwrite(spirv, sizeof(aclspv_wrd_t), (size_t)count, fp);
		fclose(fp);
	}

	free(spirv);
	return EXIT_SUCCESS;
}
