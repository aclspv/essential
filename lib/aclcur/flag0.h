#ifndef	aclcur_flag0_h
#define aclcur_flag0_h

#include <aclspv.h>

enum ACLCUR_FLAG0_ {
	/** has cursor assigned */
	ACLCUR_FLAG0_CURSOR_ASSIGNED,

	/** is expression of a value */
	ACLCUR_FLAG0_VALUE,

	/** is end of a label explicitly. */
	ACLCUR_FLAG0_LBLTERM,

	/** is end of a function explicitly. */
	ACLCUR_FLAG0_FNNTERM,

	ACLCUR_FLAG0_MAX
};

#define	aclcur_flag0_tent(CAP, sml)	\
	ae2f_inline_f ae2f_ccconst static aclspv_wrd_t sml	\
	(enum CAP c) { return 1u << (aclspv_wrd_t)(c); }	\
	typedef int sml##_check[CAP##MAX <= 32 ? 1 : -1]

aclcur_flag0_tent(ACLCUR_FLAG0_, aclcur_flag0);

#endif
