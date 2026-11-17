#ifndef	aclspv_enum_layout_h
#define aclspv_enum_layout_h

#include <aclspv/wrd.h>

/** memory layout */
enum ACLSPV_LAYOUT_ {
	ACLSPV_LAYOUT_STD430,
	ACLSPV_LAYOUT_STD140,
	ACLSPV_LAYOUT_MAX
};

/** see enum ACLSPV_LAYOUT_ */
typedef aclspv_wrd_t	aclspv_layout_t;

#endif
