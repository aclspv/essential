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

#ifndef	aclcur_h
#define aclcur_h

#include <aclspv.h>

/** id of cursor node. see aclcur_t */
typedef aclspv_id_t	aclcur_id_t;

/** 
 * physical index from the root to represent address locally on aclcur.
 * invalid value is u32 with all bit sets 1. see ACLCUR_IDX_INVAL
 * */
typedef aclspv_wrd_t		aclcur_idx_t;

/** invalid value of aclcur_idx_t */
#define	ACLCUR_IDX_INVAL	0xFFFFFFFF

/** root label value */
#define ACLCUR_IDX_ROOT		0

/** 
 * runtime value
 * has attribute of aclspv_vid_t also
 * */
typedef aclcur_id_t	aclcur_vid_t;

/**
 * runtime value but has no side effect
 * */
typedef aclcur_vid_t	aclcur_vid_pure_t;

#endif
