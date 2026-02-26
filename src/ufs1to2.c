#include "ufs/time.h"

void ufs_v1_to_v2(UFSTIMEV1 *v1, UFSTIMEV2 *v2)
{
	UFSTIMEV2	u2;
	
	// wtof("%s: v1.seconds=%08X (%u) v1->useconds=%08X (%u)",
	// 	__func__, v1->seconds, v1->seconds, v1->useconds, v1->useconds);
		
	__64_from_u32(&u2, v1->seconds);	
	__64_mul_u32(&u2, 1000, &u2);
	__64_add_u32(&u2, v1->useconds / 1000, v2);
	
	// wtof("%s: v2=%016llX (%llu)", __func__, *v2, *v2);
}
