#include <aclspv.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/** the ext-inst import table must keep its entries sorted by word count and
 * must never touch the OpExtension section (`m_ext`). */
int main(void) {
	x_aclspv_ctx		ctx;
	aclspv_usrcfg_t		cfg;
	aclspv_extinst_t	a;
	aclspv_extinst_t	b;
	aclspv_extinst_t	a2;

	memset(&cfg, 0, sizeof(cfg));
	cfg.m_flag0 = ACLSPV_CTX_FLAG0_SYMBOL;

	if(aclspv_init_ctx(&ctx, &cfg)) {
		fputs("[extinstsort] init failed\n", stderr);
		return EXIT_FAILURE;
	}

	/* "OpenCL.std" (5 words) sorts before "GLSL.std.450" (6 words), which
	 * forces the mid-insert branch on the second add. */
	a = aclspv_add_extinst_from_ctx(&ctx, "GLSL.std.450");
	b = aclspv_add_extinst_from_ctx(&ctx, "OpenCL.std");
	a2 = aclspv_add_extinst_from_ctx(&ctx, "GLSL.std.450");

	if(!a || !b || a == b || a2 != a) {
		fputs("[extinstsort] id bookkeeping is wrong\n", stderr);
		aclspv_fini_ctx(&ctx);
		return EXIT_FAILURE;
	}

	if(ctx.m_count.m_ext) {
		fputs("[extinstsort] writes leaked into the extension section\n", stderr);
		aclspv_fini_ctx(&ctx);
		return EXIT_FAILURE;
	}

	{
		const aclspv_wrd_t*	p = ctx.m_section.m_extinst.m_p;
		aclspv_wrd_t		i = 0;
		const aclspv_wrd_t	c = ctx.m_count.m_extinst;
		unsigned		num = 0;

		while(i < c) {
			const aclspv_wrd_t WC = p[i] >> 16;
			const aclspv_wrd_t OP = p[i] & 0xFFFF;

			if(!WC || i + WC > c) {
				fputs("[extinstsort] malformed import entry\n", stderr);
				aclspv_fini_ctx(&ctx);
				return EXIT_FAILURE;
			}

			if(OP != SpvOpExtInstImport) {
				fputs("[extinstsort] non-import instruction in m_extinst\n", stderr);
				aclspv_fini_ctx(&ctx);
				return EXIT_FAILURE;
			}

			++num;
			i += WC;
		}

		if(num != 2) {
			fprintf(stderr, "[extinstsort] expected 2 imports, got %u\n", num);
			aclspv_fini_ctx(&ctx);
			return EXIT_FAILURE;
		}
	}

	aclspv_fini_ctx(&ctx);
	return EXIT_SUCCESS;
}
