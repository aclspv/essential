#include <aclcursor.h>


#if	0
aclspv_bool_t aclcursor_vardecl_phi_reserve(
		aclspv_wrdcount_t	c_num_cursor,
		const h_aclcursor_t	wr_cursor,
		const h_aclspv_ctx_t	h_ctx,
		const aclspv_wrd_t	c_idx,
		const CXCursor		c_child
		) {
	while(c_num_cursor--) {
		x_aclcursor* ae2f_restrict C = &wr_cursor[c_num_cursor];
		unless(aclcursor_is_vardecl(C))	continue
	}
	return 0;
}

#endif
