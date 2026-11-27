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

/** @file execmodel.h */

#ifndef	attr_execmodel_h
#define attr_execmodel_h

#include <ae2f/cc.h>
#include <stdio.h>
#include <.aclspv/attr/rdattr.h>
#include <aclspv.h>

/** 
 * @fn static enum CXChildVisitResult attr_execmodel(CXCursor h_cur, CXCursor h_parent, enum SpvExecutionModel_* ae2f_restrict wr_data)
 * */
static enum CXChildVisitResult s_attr_execmodel(CXCursor h_cur, ae2f_unused CXCursor h_parent, CXClientData ae2f_restrict wr_data) {
	enum CXChildVisitResult	R;
	unsigned		EXECMDL;

	if(*((enum SpvExecutionModel_* ae2f_restrict)wr_data) != SpvExecutionModelMax) {
		return CXChildVisit_Break;
	}

	_aclattr_rdattr(L, NDL, h_cur, "aclspv_execmodel", R
			, CXChildVisit_Break, CXChildVisit_Recurse
			, sscanf(NDL, "aclspv_execmodel ( %u )", &EXECMDL)
			);

	if(R == CXChildVisit_Break) {
		*((enum SpvExecutionModel_* ae2f_restrict)wr_data) = (enum SpvExecutionModel_)EXECMDL;
	}

	return CXChildVisit_Recurse;
}

#endif
