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

#define aclcur_flag0_tent2(CAP, sml, ty, c_bitmax)	\
	ae2f_inline_f ae2f_ccconst static ty sml(enum CAP c) { return ((ty)1u) << (ty)(c); }	\
	typedef int sml##_check[ (CAP##MAX > (c_bitmax)) ? -1 : 1 ]

#define	aclcur_flag0_tent(CAP, sml)	\
	aclcur_flag0_tent2(CAP, sml, aclspv_wrd_t, 32)

aclcur_flag0_tent(ACLCUR_FLAG0_, aclcur_flag0);


#endif
