#include <aclspv.h>
#include <stdio.h>
#include <stdlib.h>

#include <acltest/onefile.h>
#include <acltest/compile.h>

/**
 * a carve that cannot be emitted must be REPORTED, not skipped.
 *
 * the carve lanes explain their own failures with `err_puts`, which compiles
 * away under NDEBUG (lib/util/log.h); a lane that returned ACLEXPR_CARVE_NO
 * without an error left the context clean, so the caller's aclctx_err_check
 * stayed silent, evaluation went on with the operands' values, and the module
 * came out missing the instruction.
 *
 * `s.b.c = 1` is such a lane: a member write through a nested member is
 * explicitly deferred. before aclexpr_carve raised the lane's failure as a
 * context error, this source compiled "successfully" to a module in which the
 * store simply was not there — a wrong module instead of a diagnostic.
 *
 * this is the negative counterpart of test/compile: it asserts the rejection,
 * so it writes no module and carries no spirv-val fixture.
 * */
static const char s_src_deferred[] =
"typedef int i32;\n"
"struct inner { i32 c; };\n"
"struct outer { struct inner b; };\n"
"__attribute__((annotate(\"aclspv_execmodel(5)\")))\n"
"void t_deferred(i32* out __attribute__((annotate(\"aclspv_storage_class(3)\")))) {\n"
"    struct outer s;\n"
"    s.b.c = 1;\n"
"    *out = s.b.c;\n"
"}\n";

/** the same shape with a store the emitter CAN carve, to prove the rejection
 * above is about `s.b.c = 1` and not about the function's scaffolding. */
static const char s_src_ok[] =
"typedef int i32;\n"
"__attribute__((annotate(\"aclspv_execmodel(5)\")))\n"
"void t_ok(i32* out __attribute__((annotate(\"aclspv_storage_class(3)\")))) {\n"
"    *out = 1;\n"
"}\n";

static aclspv_compile_t s_compile(
		const char*			c_src,
		aclspv_wrdcount_t* ae2f_restrict	wr_count
		) {
	struct CXUnsavedFile	files[2];
	const char*		ARG[] = {"main.c"};
	aclspv_wrd_t*		spirv = 0;
	aclspv_compile_t	RET;

	files[0].Contents	= c_src;
	files[0].Filename	= "main.c";
	files[0].Length		= (unsigned long)strlen(c_src);

	files[1].Contents	= s_oclbase;
	files[1].Filename	= "opencl-c-base.h";
	files[1].Length		= sizeof(s_oclbase) - 1;

	RET = dbg_aclspv_compile(
			files
			, 2
			, ARG
			, 1
			, wr_count
			, &spirv
			, 0
			, 0
			);

	free(spirv);

	return RET;
}

int main(void);

int main(void) {
	aclspv_wrdcount_t	count = 0;
	aclspv_compile_t	RET_OK;
	aclspv_compile_t	RET_BAD;

	acltestcfg();

	/** the scaffolding compiles: one store through a pointer. */
	RET_OK = s_compile(s_src_ok, &count);

	printf("control compile: %d (%u words)\n", (int)RET_OK, (unsigned)count);

	if(RET_OK != ACLSPV_COMPILE_OK || !count) {
		fprintf(stderr, "[carve-fail] control compile failed: %d\n"
				, (int)RET_OK);
		return EXIT_FAILURE;
	}

	/** the deferred member write must be rejected loudly. */
	RET_BAD = s_compile(s_src_deferred, &count);

	printf("deferred compile: %d\n", (int)RET_BAD);

	if(RET_BAD == ACLSPV_COMPILE_OK) {
		fputs("[carve-fail] `s.b.c = 1` was accepted; a carve that cannot be "
				"emitted is being skipped silently\n", stderr);
		return EXIT_FAILURE;
	}

	/** and it must be the carve's rejection, not an unrelated failure. */
	if(RET_BAD != ACLSPV_COMPILE_UNHANDLED_CARVE) {
		fprintf(stderr
				, "[carve-fail] rejection is %d, expected "
				  "ACLSPV_COMPILE_UNHANDLED_CARVE (%d)\n"
				, (int)RET_BAD, (int)ACLSPV_COMPILE_UNHANDLED_CARVE);
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
