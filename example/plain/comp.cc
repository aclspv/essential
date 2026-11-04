#include <aclspv.hh>
#include <iostream>
#include <fstream>
#include <assert.h>
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

int main(int argc, const char* argv[]) {
	aclspv::ctx ctx{};

	const char* outpath = argv[1];

	TEST::scoped<CXIndex, void, clang_disposeIndex> 
		cxidx(clang_createIndex(0, 0
					));

	if(argc < 3) {
		puts("comp.cc <file_output> ...<file_to_compile>");
		return -1;
	}

	argv[1] = "clang";

	TEST::scoped<CXTranslationUnit, void, clang_disposeTranslationUnit> 
		tu(clang_parseTranslationUnit(
					cxidx.get()
					, nullptr
			, argv + 1
			, argc - 1
			, 0, 0
			, 0));

	{
		aclspv_compile_t c = ctx.compile1(tu.get());
		std::cout << "output: " << c << "\n";
		T_assert(!c);
	}

	{
		aclspv::wrd_t out;
		T_assert(!ctx.dump_output(out));

		std::vector<aclspv::wrd_t> output((size_t)out);
		T_assert(!ctx.dump_output1(out, &output[0]));

		std::ofstream fs(outpath);
		if(!fs.is_open()) { fs.close(); assert(0); }

		fs.write((const char*)&output[0], sizeof(aclspv::wrd_t) * out);
	}

	return 0;
}
