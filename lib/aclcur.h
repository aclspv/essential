#ifndef	aclcur_h
#define aclcur_h

#include <aclspv.h>

/** id of cursor node. see aclcur_t */
typedef aclspv_id_t	aclcur_id_t;

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
