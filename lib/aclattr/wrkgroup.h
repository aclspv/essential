#ifndef	aclattr_wrkgroup_h
#define	aclattr_wrkgroup_h

struct aclattr_wrkgroup_prm {
	unsigned	m_0, m_1, m_2, m_flag_specid;
};


#include <aclspv.h>
#include <ae2f/cc.h>
#include <string.h>
#include <stdio.h>
#include <util/log.h>

/**
 * @param wr_data	: struct aclattr_wrkgroup
 * */
static enum CXChildVisitResult aclattr_wrkgroup(CXCursor h_cur, ae2f_unused CXCursor h_parent, CXClientData ae2f_restrict wr_data) {
	CXString	ATTR_KIND;
	const char*	NIDDLE;
#define	RETDATA	((struct aclattr_wrkgroup_prm* ae2f_restrict)(wr_data))

	ATTR_KIND = clang_getCursorSpelling(h_cur);
	if ((NIDDLE = strstr(ATTR_KIND.data, "aclspv_wrkgroup_size_id"))) {
		char F0 = '\0', F1 = '\0', F2 = '\0';
		sscanf(NIDDLE, "aclspv_wrkgroup_size_id ( %c%u , %c%u , %c%u )"
				, &F0, &RETDATA->m_0
				, &F1, &RETDATA->m_1
				, &F2, &RETDATA->m_2
		      );

		RETDATA->m_flag_specid = (unsigned)(F0 == '&') | (unsigned)((F1 == '&') << 1) | (unsigned)((F2 == '&') << 2);

		clang_disposeString(ATTR_KIND);
		return CXChildVisit_Break;
	}

	else if((NIDDLE = strstr(ATTR_KIND.data, "reqd_work_group_size"))) {

		sscanf(NIDDLE, "reqd_work_group_size ( %u , %u , %u )"
				, &RETDATA->m_0
				, &RETDATA->m_1
				, &RETDATA->m_2
		      );

		RETDATA->m_flag_specid = 0;

		clang_disposeString(ATTR_KIND);
		return CXChildVisit_Break;
	} else if ((NIDDLE = strstr(ATTR_KIND.data, "aclspv_wrkgroup_size"))) {
		sscanf(NIDDLE, "aclspv_wrkgroup_size ( %u , %u , %u )"
				, &RETDATA->m_0
				, &RETDATA->m_1
				, &RETDATA->m_2
		      );

		RETDATA->m_flag_specid = 0;

		clang_disposeString(ATTR_KIND);
		return CXChildVisit_Break;
	}

	clang_disposeString(ATTR_KIND);
	return CXChildVisit_Continue;
#undef	RETDATA
}

#endif
