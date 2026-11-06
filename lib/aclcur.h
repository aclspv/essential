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
