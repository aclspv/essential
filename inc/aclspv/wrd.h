#ifndef	aclspv_wrd_h
#define aclspv_wrd_h

#include <ae2f/c90/stdint.h>

/**
 * @typedef	aclspv_wrd_t
 * @brief	integer as word.
 * 		could be defined with `#define`
 * 		32-bits unsigned.
 * */
#ifndef	aclspv_wrd_t
typedef uint_least32_t	aclspv_wrd_t;
#define	aclspv_wrd_t	aclspv_wrd_t
#endif

typedef int ACLSPV_WRD_CHECK[sizeof(aclspv_wrd_t) >= 4 ? 1 : -1];

/**
 * 64-bits unsigned.
 * */
#ifndef	aclspv_wrd2_t
typedef	uint_least64_t	aclspv_wrd2_t;
#define	aclspv_wrd2_t	aclspv_wrd2_t
#endif

#ifndef	aclspv_swrd2_t
typedef	int_least64_t	aclspv_swrd2_t;
#define	aclspv_swrd2_t	aclspv_swrd2_t
#endif

typedef aclspv_wrd2_t	aclspv_wrdmax_t;
typedef aclspv_swrd2_t	aclspv_swrdmax_t;

/**
 * @typedef	aclspv_wrdcount_t
 * @brief	the integer type represents the number of word.
 * */
typedef aclspv_wrd_t	aclspv_wrdcount_t;

#endif
