#include <aclspv.h>
#include <stdio.h>
#include <stdlib.h>

#include <acltest/onefile.h>
#include <acltest/compile.h>
#include <acltest/spirv.h>

/**
 * the synchronisation builtins: `aclspv_ctlbar` -> OpControlBarrier and
 * `aclspv_membar` -> OpMemoryBarrier.
 *
 * the source declares both, as <aclspv.h> does, but they have no body: the
 * call is recognised by spelling and lowered in place. that gives four things
 * to check here:
 *
 *   - the instruction has no Result Type and no Result:
 *       OpControlBarrier [op|4][exec scope][mem scope][semantics]
 *       OpMemoryBarrier  [op|3][mem scope][mem semantics]
 *     a value-shaped emission would make the word count 6 / 5.
 *   - its operands are ids, and each one is an OpConstant of the u32
 *     parameter type holding the value written at the call site.
 *   - the declared prototype did not become a dangling import: the module
 *     holds exactly the one OpFunction, the entry point.
 *
 * the parameters are plain Output pointers on purpose: the case is about the
 * barriers, and a Workgroup (`storage_class(4)`) parameter goes through an
 * unrelated struct-wrapping path.
 * */
static const char s_src_sync[] =
"typedef unsigned int u32;\n"
"void aclspv_ctlbar(u32 e, u32 m, u32 s);\n"
"void aclspv_membar(u32 m, u32 s);\n"
"__attribute__((annotate(\"aclspv_execmodel(5)\")))\n"
"void t_sync(\n"
"    __attribute__((annotate(\"aclspv_storage_class(3)\"))) u32* out\n"
") {\n"
"    u32 v = *out;\n"
"    aclspv_ctlbar(2, 2, 264);\n"
"    *out = v;\n"
"    aclspv_membar(2, 264);\n"
"}\n";

/** the words of the first instruction with opcode `c_op`, or -1 when there is
 * none or its word count is not `c_wr`. */
static int s_first_op(
		const aclspv_wrd_t*	rd,
		aclspv_wrdcount_t	c,
		unsigned		c_op,
		aclspv_wrd_t* ae2f_restrict	r_wr,
		unsigned		c_wr
		) {
	aclspv_wrdcount_t i = 5;

	while(i < c) {
		const unsigned wc = (unsigned)(rd[i] >> 16);
		unsigned j;

		if(!wc || i + wc > c)
			return -1;

		if((unsigned)(rd[i] & 0xffff) == c_op) {
			if(wc != c_wr)
				return -1;

			for(j = 0; j < c_wr; ++j)
				r_wr[j] = rd[i + j];

			return 0;
		}

		i += wc;
	}

	return -1;
}

/** the value of `c_id` as an OpConstant of a 32-bit unsigned integer.
 *
 * returns 0 and stores the value when it is exactly that, -1 otherwise. a
 * barrier operand emitted as a literal word instead of an id would land here
 * as an unresolvable id, which is what this rejects.
 * */
static int s_const_u32(
		const aclspv_wrd_t*	rd,
		aclspv_wrdcount_t	c,
		aclspv_wrd_t		c_id,
		aclspv_wrd_t* ae2f_restrict	r_val
		) {
	aclspv_wrdcount_t	i = 5;
	aclspv_wrd_t		ty = 0;

	if(!c_id)
		return -1;

	while(i < c) {
		const unsigned wc = (unsigned)(rd[i] >> 16);
		const unsigned op = (unsigned)(rd[i] & 0xffff);

		if(!wc || i + wc > c)
			return -1;

		/** OpTypeInt [op|4][result][width][signedness] */
		if(op == (unsigned)SpvOpTypeInt && wc == 4
				&& rd[i + 2] == 32u && rd[i + 3] == 0u)
			ty = rd[i + 1];

		/** OpConstant [op|4][result type][result][value] */
		if(op == (unsigned)SpvOpConstant && wc == 4
				&& rd[i + 2] == c_id) {
			if(!ty || rd[i + 1] != ty)
				return -1;

			*r_val = rd[i + 3];
			return 0;
		}

		i += wc;
	}

	return -1;
}

/** `c_num` operands of `WR` (from index 1) must be the given values. */
static int s_check_operands(
		const char*		c_what,
		const aclspv_wrd_t*	rd,
		aclspv_wrdcount_t	c,
		const aclspv_wrd_t*	WR,
		unsigned		c_num,
		const aclspv_wrd_t*	c_want
		) {
	unsigned i;

	for(i = 0; i < c_num; ++i) {
		aclspv_wrd_t VAL;

		if(s_const_u32(rd, c, WR[1 + i], &VAL) != 0) {
			fprintf(stderr
					, "[sync] %s operand %u is not an unsigned constant\n"
					, c_what, i);
			return -1;
		}

		if(VAL != c_want[i]) {
			fprintf(stderr
					, "[sync] %s operand %u is %u, expected %u\n"
					, c_what, i, (unsigned)VAL, (unsigned)c_want[i]);
			return -1;
		}
	}

	return 0;
}

int main(void);

int main(void) {
	struct CXUnsavedFile	files[2];
	const char*		ARG[] = {"main.c"};
	aclspv_compile_t	RET;
	aclspv_wrd_t*		spirv = 0;
	aclspv_wrdcount_t	count = 0;
	acltest_spirv_stats	stats;
	unsigned		c_ctl;
	unsigned		c_mem;
	unsigned		c_fn;
	aclspv_wrd_t		WR[4];

	/** the words written at the call sites, in instruction order. */
	static const aclspv_wrd_t CTL[] = { 2u, 2u, 264u };
	static const aclspv_wrd_t MEM[] = { 2u, 264u };

	acltestcfg();

	files[0].Contents	= s_src_sync;
	files[0].Filename	= "main.c";
	files[0].Length		= sizeof(s_src_sync) - 1;

	files[1].Contents	= s_oclbase;
	files[1].Filename	= "opencl-c-base.h";
	files[1].Length		= sizeof(s_oclbase) - 1;

	RET = dbg_aclspv_compile(
			files
			, 2
			, ARG
			, 1
			, &count
			, &spirv
			, 0
			, 0
			);

	printf("aclspv_compile: %d\n", (int)RET);
	printf("output spirv: %p, %u\n", (void*)spirv, count);

	if(RET != ACLSPV_COMPILE_OK) {
		fprintf(stderr, "[sync] compile failed: %d\n", (int)RET);
		return EXIT_FAILURE;
	}

	if(!spirv) {
		fputs("[sync] compile produced no spir-v\n", stderr);
		return EXIT_FAILURE;
	}

	if(acltest_spirv_scan(spirv, count, &stats) != 0) {
		fputs("[sync] malformed spir-v instruction stream\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	c_ctl = acltest_spirv_count_op(spirv, count, (unsigned)SpvOpControlBarrier);
	c_mem = acltest_spirv_count_op(spirv, count, (unsigned)SpvOpMemoryBarrier);
	c_fn  = acltest_spirv_count_op(spirv, count, (unsigned)SpvOpFunction);

	printf("ctlbar=%u membar=%u fn=%u\n", c_ctl, c_mem, c_fn);

	if(c_ctl != 1 || c_mem != 1) {
		fprintf(stderr
				, "[sync] barriers: ctlbar %u, membar %u (expected 1 each)\n"
				, c_ctl, c_mem);
		free(spirv);
		return EXIT_FAILURE;
	}

	/** the declared prototypes must not have become imports: a call to a
	 * body-less declaration would add an OpFunction with no block. */
	if(c_fn != 1) {
		fprintf(stderr
				, "[sync] functions: %u, expected 1 (the entry point)\n"
				, c_fn);
		free(spirv);
		return EXIT_FAILURE;
	}

	/** the word counts below are what "no Result Type, no Result" means: a
	 * value-shaped emission would carry two more words. */
	if(s_first_op(spirv, count, (unsigned)SpvOpControlBarrier, WR, 4) != 0) {
		fputs("[sync] OpControlBarrier is not [op|4][3 words]\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	if(s_check_operands("OpControlBarrier", spirv, count, WR, 3, CTL) != 0) {
		free(spirv);
		return EXIT_FAILURE;
	}

	if(s_first_op(spirv, count, (unsigned)SpvOpMemoryBarrier, WR, 3) != 0) {
		fputs("[sync] OpMemoryBarrier is not [op|3][2 words]\n", stderr);
		free(spirv);
		return EXIT_FAILURE;
	}

	if(s_check_operands("OpMemoryBarrier", spirv, count, WR, 2, MEM) != 0) {
		free(spirv);
		return EXIT_FAILURE;
	}

	{
		FILE* const fp = fopen(ACLTEST_OUT, "wb");

		if(!fp) {
			fprintf(stderr, "[sync] cannot write %s\n", ACLTEST_OUT);
			free(spirv);
			return EXIT_FAILURE;
		}

		fwrite(spirv, sizeof(aclspv_wrd_t), (size_t)count, fp);
		fclose(fp);
	}

	free(spirv);
	return EXIT_SUCCESS;
}
