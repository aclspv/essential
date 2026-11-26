#include <aclspv.hh>
#include <iostream>
#include <fstream>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <vector>

namespace TEST {
	template<typename T, typename R> using disposal_t = R(T);

	template<typename T, typename R, disposal_t<T, R> disposal>
	class scoped {
		T pm;

		public:
		scoped(T t) : pm(t) { assert(t); }
		~scoped() { disposal(this->pm); }
		T get(void) const { return this->pm; }
	};
}

#define abortmsg(msg) std::cerr << "ABORTION: \n\t" #msg "\n\t" __FILE__ ":" <<__LINE__ << '\n'
#define T_assert(c) if(!(c)) { abortmsg(c); return -2; }

static void s_usage(const char* c_argv0) {
	fprintf(stderr,
			"usage: %s [--flag MASK] <spirv_out> <file_to_compile> [clang args..]\n"
			"\n"
			"  --flag MASK   ACLSPV_CTX_FLAG0_ bits, hex (0x104) or decimal (260).\n"
			"                repeatable; the masks are OR-ed.\n"
			"                the bits are the ones in inc/aclspv/usrcfg.h,\n"
			"                e.g. 0x001 SYMBOL, 0x080 EXPORT_BY_DEFAULT,\n"
			"                     0x100 INLINE_BY_DEFAULT.\n"
			"\n"
			"ACLSPV_CTX_FLAG0_SYMBOL is always on: the writer needs names.\n"
			, c_argv0);
}

/** one `--flag` argument: a number, hex (`0x..`) or decimal. 0 means "no
 * bits", so a bare `--flag 0` is rejected as a missing value. */
static aclspv::wrd_t s_flag_parse(const char* c_arg) {
	char*			c_end = 0;
	const unsigned long	V = strtoul(c_arg, &c_end, 0);

	if(!c_end || c_end == c_arg || *c_end)
		return 0;

	return (aclspv::wrd_t)V;
}

int main(int argc, const char* argv[]) {
	aclspv::wrd_t flags = ACLSPV_CTX_FLAG0_SYMBOL;
	int argi = 1;

	if(argc < 3) {
		s_usage(argv[0]);
		return -1;
	}

	while(argi < argc && !strncmp(argv[argi], "--flag", 6)) {
		if(argv[argi][6]) {
			s_usage(argv[0]);
			return -1;
		}

		if(argi + 1 >= argc) {
			fprintf(stderr, "comp.cc: --flag needs a value\n");
			return -1;
		}

		{
			const aclspv::wrd_t c_bits = s_flag_parse(argv[argi + 1]);

			if(!c_bits) {
				s_usage(argv[0]);
				return -1;
			}

			flags |= c_bits;
		}

		argi += 2;
	}

	if(argc - argi < 2) {
		s_usage(argv[0]);
		return -1;
	}

	{
		const char* outpath = argv[argi];
		aclspv::ctx ctx{};
		TEST::scoped<CXIndex, void, clang_disposeIndex>
			cxidx(clang_createIndex(0, 0
						));

		ctx.me().m_usrcfg.m_flag0 = (aclspv::wrd_t)flags;

		/* the output path is not a source: hand every remaining argument to
		 * clang, mirrored as the driver it would have been. */
		std::vector<const char*> clang_argv(argc - argi - 1);
		for(int i = 0; i < argc - argi - 1; ++i)
			clang_argv[(size_t)i] = argv[argi + 1 + (size_t)i];

		TEST::scoped<CXTranslationUnit, void, clang_disposeTranslationUnit>
			tu(clang_parseTranslationUnit(
						cxidx.get()
						, nullptr
						, &clang_argv[0]
						, (int)clang_argv.size()
						, 0, 0
						, 0));

		{
			aclspv_compile_t c = ctx.build1(tu.get());
			std::cout << "output: " << c << "\n";
			T_assert(!c);
		}

		{
			aclspv::wrd_t out;
			T_assert(!ctx.dump_output(out));

			std::vector<aclspv::wrd_t> output((size_t)out);
			T_assert(!ctx.dump_output1(out, &output[0]));

			std::ofstream fs(outpath, std::ios::binary);
			if(!fs.is_open()) { fs.close(); assert(0); }

			fs.write((const char*)&output[0], sizeof(aclspv::wrd_t) * out);
		}
	}

	return 0;
}
