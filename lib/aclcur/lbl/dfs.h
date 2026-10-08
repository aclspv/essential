#ifndef	ACLCUR_LBL_DFS_H
#define ACLCUR_LBL_DFS_H

#include "../class.h"

enum ACLCUR_LBL_DFS_ {
	ACLCUR_LBL_DFS_CONTINUE,
	ACLCUR_LBL_DFS_BREAK,
	ACLCUR_LBL_DFS_IN
};

typedef enum ACLCUR_LBL_DFS_ aclcur_lbl_dfs_t(
		h_aclcur_t	h_child,
		h_aclcur_t	h_par_opt,
		void*		rdwr_client
		);

enum ACLCUR_LBL_DFS_ aclcur_lbl_dfs(
		aclspv_id_t		c_lbl,
		aclcur_lbl_dfs_t	fn,
		void*			rdwr_client,
		h_aclspv_ctx_t		h_ctx
		);

#endif
