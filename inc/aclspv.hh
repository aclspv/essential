#ifndef aclspv_hh
#define aclspv_hh

#include "./aclspv.h"
#include <ae2f/bll.h>
#include <ae2f/c90/stdint/enum.h>

#include <ae2f/cxx03/constexprmethod.h>
#include <ae2f/cxx03/constexprfun.h>
#include <ae2f/cxx03/rmov.h>

namespace aclspv {
	typedef aclspv_wrd_t		wrd_t;
	typedef aclspv_wrdcount_t	wrdcount_t;
	typedef aclspv_ver_vk_t		ver_vk_t;
	typedef aclspv_ver_spv_t	ver_spirv_t;

	ae2f_constexprfun ae2f_inline_f ae2f_ccconst static ver_spirv_t ver_spv(const wrd_t c_mjr, const wrd_t c_mnr) {
		return aclspv_ver_spv(c_mjr, c_mnr);
	}

	ae2f_constexprfun ae2f_inline_f ae2f_ccconst static ver_vk_t ver_vk(
			const wrd_t c_var, const wrd_t c_mjr,
			const wrd_t c_mnr, const wrd_t c_patch
			) {
		return aclspv_ver_vk(c_var, c_mjr, c_mnr, c_patch);
	}

	ae2f_constexprfun ae2f_inline_f ae2f_ccconst static ver_vk_t ver_vk(
			const wrd_t c_mjr, const wrd_t c_mnr
			) {
		return aclspv_ver_vk2(c_mjr, c_mnr);
	}

	struct ctx_base : protected a_aclspv_ctx {
		public:
			ae2f_constexprmethod ae2f_inline_f a_aclspv_ctx& ae2f_ccconst me() {
				return *this;
			}

			ae2f_constexprmethod ae2f_inline_f const a_aclspv_ctx& ae2f_ccconst c_me() const {
				return *this;
			}

			ae2f_inline_f aclspv_compile_t 
				dump_output1(wrd_t &ae2f_restrict r_output_count,
						wrd_t *ae2f_restrict const r_output) const {
					return aclspv_dump_ctx_output1(
							this
							, &r_output_count
							, r_output
							);
				}

			ae2f_inline_f aclspv_compile_t
				dump_output(wrd_t &ae2f_restrict r_output_count) const {
					return aclspv_dump_ctx_output(
							this
							, &r_output_count
							, 0
							);
				}

			ae2f_inline_f aclspv_compile_t
				dump_output(wrdcount_t &ae2f_restrict r_output_count,
						wrd_t * ae2f_restrict & ae2f_restrict r_output) const {
					return aclspv_dump_ctx_output(
							this
							, &r_output_count
							, &r_output
							);
				}

			ae2f_inline_f aclspv_compile_t
				compile(const CXCursor c_cursor) {
					return aclspv_ctx_compile(this, c_cursor);
				}

			ae2f_inline_f aclspv_compile_t
				compile1(const CXTranslationUnit c_cxtu) {
					return aclspv_ctx_compile1(this, c_cxtu);
				}

			ae2f_inline_f aclspv_compile_t compile2(
					struct CXUnsavedFile *ae2f_restrict const rdwr_unsaved,
					const unsigned c_unsaved_count,
					const char *ae2f_restrict const *ae2f_restrict const rd_argv_opt,
					const int c_argc, enum CXErrorCode *ae2f_restrict rwr_cxerr_opt,
					CXIndex h_cxidx) {
				return aclspv_ctx_compile2(this, rdwr_unsaved, c_unsaved_count,
						rd_argv_opt, c_argc, rwr_cxerr_opt,
						h_cxidx);
			}

			ae2f_inline_f aclspv_compile_t compile3(
					struct CXUnsavedFile *ae2f_restrict const rdwr_unsaved,
					const unsigned c_unsaved_count,
					const char *ae2f_restrict const *ae2f_restrict const rd_argv_opt,
					const int c_argc, enum CXErrorCode *ae2f_restrict rwr_cxerr_opt) {
				return aclspv_ctx_compile3(this, rdwr_unsaved, c_unsaved_count,
						rd_argv_opt, c_argc, rwr_cxerr_opt);
			}

			ae2f_inline_f ae2f_ccpure aclspv_bool_t	chck_capability(
					const enum SpvCapability_		c_cap
					) const {
				return aclspv_chck_capability_from_ctx(this, c_cap);
			}

			ae2f_inline_f aclspv_bool_t	add_capability(
					const enum SpvCapability_		c_cap
					) {
				return aclspv_add_capability_from_ctx(this, c_cap);
			}

			ae2f_inline_f ae2f_ccpure aclspv_bool_t	chck_extension(
					const char* const	rd_ext
					) const {
				return aclspv_chck_extension_from_ctx(this, rd_ext);;
			}

			ae2f_inline_f aclspv_bool_t	add_extension(
					const char* const	rd_ext
					) {
				return aclspv_add_extension_from_ctx(this, rd_ext);
			}

		protected:
			ae2f_inline_f ctx_base() {}
	};

	typedef enum ACLSPV_CTX_FLAG0_ CTX_FLAG0_;

	struct usrcfg_t : public aclspv_usrcfg_t {
		ae2f_constexprmethod ae2f_inline_f usrcfg_t() : aclspv_usrcfg_t{} {}
		ae2f_constexprmethod ae2f_inline_f usrcfg_t(const usrcfg_t& c) : aclspv_usrcfg_t(c) {}
		ae2f_constexprmethod ae2f_inline_f usrcfg_t(const usrcfg_t&& c) : aclspv_usrcfg_t(c) {}
		ae2f_constexprmethod ae2f_inline_f usrcfg_t(const aclspv_usrcfg_t& c) : aclspv_usrcfg_t(c) {}
		ae2f_constexprmethod ae2f_inline_f usrcfg_t(const aclspv_usrcfg_t&& c) : aclspv_usrcfg_t(c) {}

		typedef ae2f_rty(aclspv::usrcfg_t) rty;

		ae2f_constexprmethod ae2f_inline_f ae2f_ccpure wrd_t spirv_ver(void) const { return this->m_ver_spv; }
		ae2f_constexprmethod ae2f_inline_f ae2f_ccpure rty spirv_ver(ver_spirv_t c) {
			rty r = ae2f_rmov(*this);
			r.m_ver_spv = c;
			return ae2f_rmov(r);
		}
		ae2f_constexprmethod ae2f_inline_f ae2f_ccpure rty spirv_ver(wrd_t c_mjr, wrd_t c_mnr) {
			return this->spirv_ver(ver_spv(c_mjr, c_mnr));
		}

		ae2f_constexprmethod ae2f_inline_f ae2f_ccpure rty flag0(wrd_t c) {
			rty r = ae2f_rmov(*this);
			r.m_flag0 = c;
			return ae2f_rmov(r);
		}

		ae2f_constexprmethod ae2f_inline_f ae2f_ccpure wrd_t flag0(void) const {
			return this->m_flag0;
		}

		ae2f_constexprmethod ae2f_inline_f ae2f_ccpure rty mdl_addr(wrd_t c) {
			ae2f_rty(aclspv::usrcfg_t) r = ae2f_rmov(*this);
			r.m_mdl_addr = c;
			return ae2f_rmov(r);
		}

		ae2f_constexprmethod ae2f_inline_f ae2f_ccpure rty mdl_addr(SpvAddressingModel c) {
			return mdl_addr(static_cast<wrd_t>(c));
		}

		ae2f_constexprmethod ae2f_inline_f ae2f_ccpure rty mdl_mem(wrd_t c) {
			ae2f_rty(aclspv::usrcfg_t) r = ae2f_rmov(*this);
			r.m_mdl_mem = c;
			return ae2f_rmov(r);
		}

		ae2f_constexprmethod ae2f_inline_f ae2f_ccpure rty mdl_mem(SpvMemoryModel c) {
			return this->mdl_mem(static_cast<wrd_t>(c));
		}
	};

	struct ctx : public ctx_base {
		public:
			ae2f_inline_f ctx(void) {
				aclspv_compile_t A = aclspv_init_ctx(
						this, 0);

				this->m_err = this->m_err ? this->m_err : A;
			}

			ae2f_inline_f ctx(const usrcfg_t& cfg) {
				aclspv_compile_t A = aclspv_init_ctx(
						this, &cfg);

				this->m_err = this->m_err ? this->m_err : A;
			}

			ae2f_inline_f ctx(const usrcfg_t&& cfg) {
				aclspv_compile_t A = aclspv_init_ctx(
						this, &cfg);

				this->m_err = this->m_err ? this->m_err : A;
			}

			~ctx() { aclspv_fini_ctx(this); }
	};
}

#endif
