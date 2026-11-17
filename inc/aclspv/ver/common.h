#ifndef	aclspv_ver_common_h
#define aclspv_ver_common_h

#include <ae2f/Cast.h>

/** ((c & c_and) << c_pad) */
#define aclspv_and_pad(c, c_and, c_pad)	\
	((ae2f_static_cast(aclspv_wrd_t, (c)) & ae2f_static_cast(aclspv_wrd_t, (c_and))) \
	 << ae2f_static_cast(aclspv_wrd_t, (c_pad)))

#endif
