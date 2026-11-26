#include <aclspv.h>
#include <stdio.h>
#include <stdlib.h>
#include <acltest/onefile.h>
#include <acltest/compile.h>
#include <acltest/spirv.h>

static const char s_inline[] =
"struct A { int x; A(){x=1;} ~A(){x=0;} };\n"
"struct B { A a; A arr[2]; int y; B(){y=2;} ~B(){y=3;} };\n"
"__attribute__((annotate(\"aclspv_execmodel(0)\")))\n"
"void entry(void) { B b{}; }\n";

static const char s_noinline[] =
"struct A { int x; A(); ~A(); };\n"
"A::A(){x=1;} A::~A(){x=0;}\n"
"struct B { A a; A arr[2]; B(){} ~B(){} };\n"
"__attribute__((annotate(\"aclspv_execmodel(0)\")))\n"
"void entry(void) { B b{}; }\n";


static unsigned s_count_dtor_calls(const aclspv_wrd_t* rd, aclspv_wrdcount_t n)
{
	aclspv_wrd_t ids[32];
	unsigned nid = 0, calls = 0;
	aclspv_wrdcount_t i = 5;
	while(i < n) {
		unsigned wc = (unsigned)(rd[i] >> 16), op = (unsigned)(rd[i] & 0xffff);
		if(!wc || i + wc > n) break;
		if(op == 5 && wc >= 3) { /* OpName */
			const char* nm = (const char*)&rd[i + 2];
			if(strstr(nm, "D1") && nid < 32) ids[nid++] = rd[i + 1];
		}
		i += wc;
	}
	i = 5;
	while(i < n) {
		unsigned wc = (unsigned)(rd[i] >> 16), op = (unsigned)(rd[i] & 0xffff);
		if(!wc || i + wc > n) break;
		if(op == 57 && wc >= 4) { /* OpFunctionCall */
			unsigned j; for(j = 0; j < nid; ++j) if(rd[i + 3] == ids[j]) { ++calls; break; }
		}
		i += wc;
	}
	return calls;
}

static const char s_move[] =
"struct S { int x; S(){x=1;} S(S&& o){x=o.x;} ~S(){x=0;} int get() const { return x; } };\n"
"namespace std { S&& move(S& o) { return static_cast<S&&>(o); } }\n"
"__attribute__((annotate(\"aclspv_execmodel(0)\")))\n"
"void entry(void) { S a{}; S b=std::move(a); S c{}; int y=b.get()+c.get(); }\n";

static const char s_mixed[] =
"struct S { int x; S(){x=1;} S(const S& o){x=o.x+1;} ~S(){x=0;} int get() const { return x; } };\n"
"void byval(S s) { int z=s.get(); }\n"
"__attribute__((annotate(\"aclspv_execmodel(0)\")))\n"
"void entry(void) { S a{}; S b=a; byval(a); int y=b.get(); }\n";

static aclspv_compile_t s_compile(const char* src, aclspv_wrd_t** out, aclspv_wrdcount_t* n)
{
	struct CXUnsavedFile files[2];
	const char* ARG[] = {"main.cc"};
	acltestcfg();
	files[0].Contents=src; files[0].Filename="main.cc"; files[0].Length=(unsigned long)strlen(src);
	files[1].Contents=s_oclbase; files[1].Filename="opencl-c-base.h"; files[1].Length=sizeof(s_oclbase)-1;
	return dbg_aclspv_compile(files,2,ARG,1,n,out,0,ACLSPV_CTX_FLAG0_SYMBOL);
}

int main(void)
{
	aclspv_wrd_t* spv = 0; aclspv_wrdcount_t n = 0; acltest_spirv_stats stats;
	aclspv_compile_t r = s_compile(s_inline, &spv, &n);
	if(r != ACLSPV_COMPILE_OK || !spv) { fprintf(stderr,"inline subdtor failed %d\n",(int)r); free(spv); return EXIT_FAILURE; }
	fprintf(stderr,"inline dtor_calls=%u\n", s_count_dtor_calls(spv,n));
	if(s_count_dtor_calls(spv,n) > 1) { fprintf(stderr,"inlined subobject still calls dtor\n"); free(spv); return EXIT_FAILURE; }
	printf("inline subdtor ok, words=%u\n",(unsigned)n);
	{ FILE* fp = fopen(ACLTEST_OUT, "wb"); if(fp) { fwrite(spv, sizeof(*spv), (size_t)n, fp); fclose(fp); } }
	free(spv);
	r = s_compile(s_noinline, &spv, &n);
	if(r != ACLSPV_COMPILE_SUBOBJECT_DTOR_NOT_INLINE) { fprintf(stderr,"non-inline subdtor not rejected: %d\n",(int)r); free(spv); return EXIT_FAILURE; }
	printf("non-inline subdtor rejected: %d\n",(int)r);
	free(spv);

	r = s_compile(s_mixed, &spv, &n);
	if(r != ACLSPV_COMPILE_OK || !spv) { fprintf(stderr,"mixed byvalue failed %d\n",(int)r); free(spv); return EXIT_FAILURE; }
	if(acltest_spirv_scan(spv,n,&stats)) { fputs("mixed bad spir-v\n", stderr); free(spv); return EXIT_FAILURE; }
	if(s_count_dtor_calls(spv,n) != 3) {
		fputs("mixed byvalue/dtor call count mismatch\n", stderr); free(spv); return EXIT_FAILURE;
	}
	printf("mixed byvalue dtor ok, dtor_calls=%u\n", s_count_dtor_calls(spv,n));
	free(spv);

	r = s_compile(s_move, &spv, &n);
	if(r != ACLSPV_COMPILE_OK || !spv) { fprintf(stderr,"move dtor test failed %d\n",(int)r); free(spv); return EXIT_FAILURE; }
	if(s_count_dtor_calls(spv,n) != 2) { fputs("std::move dtor set mismatch\n", stderr); free(spv); return EXIT_FAILURE; }
	printf("std::move dtor suppression ok, dtor_calls=2\n");
	free(spv);
	return EXIT_SUCCESS;
}
