/*
 * Copyright 2026 dalmurii
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * See the LICENSE file for the full text, including LLVM Exceptions.
 */

#ifndef	ACLCUR_LBL_DFS_H
#define ACLCUR_LBL_DFS_H

#include "../class.h"
#include "./id.h"

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
		aclcur_lblid_t		c,
		aclcur_lbl_dfs_t	fn,
		void*			rdwr_client,
		h_aclspv_ctx_t		h_ctx
		);

#endif
