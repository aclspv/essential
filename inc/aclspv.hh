#ifndef aclspv_hh
#define aclspv_hh

#include "./aclspv.h"
#include <ae2f/bll.h>
#include <ae2f/c90/StdInt/enum.h>
#include <clang-c/Index.h>

namespace aclspv {
typedef aclspv_wrd_t wrd_t;
typedef aclspv_wrdcount_t wrdcount_t;

struct ctx_base : protected a_aclspv_ctx {
      public:
	ae2f_always_inline enum ACLSPV_COMPILE_
	dump_output1(wrd_t &ae2f_restrict r_output_count,
		     wrd_t *ae2f_restrict const r_output) const {
		return aclspv_dump_ctx_output1(this, &r_output_count, r_output);
	}

	ae2f_always_inline enum ACLSPV_COMPILE_
	dump_output(wrd_t &ae2f_restrict r_output_count) const {
		return aclspv_dump_ctx_output(this, &r_output_count, 0);
	}

	ae2f_always_inline enum ACLSPV_COMPILE_
	dump_output(wrdcount_t &ae2f_restrict r_output_count,
		    wrd_t * ae2f_restrict & ae2f_restrict r_output) const {
		return aclspv_dump_ctx_output(this, &r_output_count, &r_output);
	}

	ae2f_always_inline enum ACLSPV_COMPILE_
	compile(const CXCursor c_cursor) {
		return aclspv_ctx_compile(this, c_cursor);
	}

	ae2f_always_inline enum ACLSPV_COMPILE_
	compile1(const CXTranslationUnit c_cxtu) {
		return aclspv_ctx_compile1(this, c_cxtu);
	}

	ae2f_always_inline enum ACLSPV_COMPILE_ compile2(
	    struct CXUnsavedFile *ae2f_restrict const rdwr_unsaved,
	    const unsigned c_unsaved_count,
	    const char *ae2f_restrict const *ae2f_restrict const rd_argv_opt,
	    const int c_argc, enum CXErrorCode *ae2f_restrict rwr_cxerr_opt,
	    CXIndex h_cxidx) {
		return aclspv_ctx_compile2(this, rdwr_unsaved, c_unsaved_count,
					   rd_argv_opt, c_argc, rwr_cxerr_opt,
					   h_cxidx);
	}

	ae2f_always_inline enum ACLSPV_COMPILE_ compile3(
	    struct CXUnsavedFile *ae2f_restrict const rdwr_unsaved,
	    const unsigned c_unsaved_count,
	    const char *ae2f_restrict const *ae2f_restrict const rd_argv_opt,
	    const int c_argc, enum CXErrorCode *ae2f_restrict rwr_cxerr_opt) {
		return aclspv_ctx_compile3(this, rdwr_unsaved, c_unsaved_count,
					   rd_argv_opt, c_argc, rwr_cxerr_opt);
	}

      protected:
	ae2f_always_inline ctx_base() = default;
};

struct ctx : public ctx_base {
      public:
	ae2f_always_inline ctx(const wrd_t c_init_size_exp,
			       const wrd_t c_init_flag0,
			       const wrd_t c_spirv_version) {
		e_aclspv_compile_t A = aclspv_init_ctx(
		    this, c_init_size_exp, c_init_flag0, c_spirv_version);

		this->m_err = this->m_err ? this->m_err : A;
	}

	~ctx() { aclspv_fini_ctx(this); }
};
}; // namespace aclspv

#endif
