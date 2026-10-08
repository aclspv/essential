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
