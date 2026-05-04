#ifndef	aclspv_hh
#define aclspv_hh

#include "./aclspv.h"
#include <ae2f/bll.h>

namespace aclspv {
	typedef aclspv_wrd_t wrd_t;
	typedef aclspv_wrdcount_t wrdcount_t;

	struct ctx_base : protected a_aclspv_ctx {
		public:
			ae2f_always_inline enum ACLSPV_COMPILE_ dump_output(
					wrd_t& ae2f_restrict	r_output_count
					) {
				return aclspv_dump_ctx_output(this
						, &r_output_count
						, 0 
						);
			}

			ae2f_always_inline enum ACLSPV_COMPILE_ dump_output(
					wrdcount_t& ae2f_restrict		r_output_count,
					wrd_t* ae2f_restrict& ae2f_restrict	r_output
					) const {
				return aclspv_dump_ctx_output(
						this
						, &r_output_count
						, &r_output
						);
			}

			ae2f_always_inline enum ACLSPV_COMPILE_ compile(
					const CXCursor	c_cursor
					) {
				return aclspv_ctx_compile(this, c_cursor);
			}

		protected:
			ae2f_always_inline ctx_base() = default;
	};

	struct ctx : public ctx_base {
		public:
			ae2f_always_inline ctx(
					const wrd_t c_init_size_exp,
					const wrd_t c_init_flag0,
					const wrd_t c_spirv_version
					) {
				enum ACLSPV_COMPILE_ A = aclspv_init_ctx(
						this
						, c_init_size_exp
						, c_init_flag0
						, c_spirv_version);

				this->m_err = ae2f_static_cast(
						enum ACLSPV_COMPILE_
						, ae2f_bll_sel_bool_naive1(
							uint_leastenum_t
							, this->m_err
							, this->m_err
							, A)
						);
			}

			~ctx() {
				aclspv_fini_ctx(this);
			}
	};
};

#endif
