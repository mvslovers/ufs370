#ifndef UFS_TIME_H
#define UFS_TIME_H

#include <time.h>               /* standard library time    */
#include <time64.h>
#include <socket.h>             /* has struct timeval, go figure! */
#include "ufs/types.h"          /* our atomic types         */

#ifndef TYPE_UFSTIMEV
#define TYPE_UFSTIMEV
typedef struct ufs_timeval1
{
    UINT32  seconds;
    UINT32  useconds;
} UFSTIMEV1;

typedef utime64_t UFSTIMEV2;

typedef union ufs_timeval
{
	UFSTIMEV1	v1;	/* legacy time value v1.useconds < 1000000 */
	UFSTIMEV2	v2;	/* latest time value time64_t 64 bit value */
} UFSTIMEV;
#endif

#if 0 /* deprecated */
/* get GMT time as double (secs.usecs) value with Unix epoch (dsecs can be NULL) */
DSECS ufs_time(DSECS *dsecs)                                        asm("UFSDSECS");
DSECS ufs_dsecs(DSECS *dsecs)                                       asm("UFSDSECS");
#endif

/* get GMT time as timeval value with Unix epoch (tv can be NULL)*/
UFSTIMEV ufs_timeval(UFSTIMEV *tv)                                  asm("UFSDSTV");

void ufs_v1_to_v2(UFSTIMEV1 *v1, UFSTIMEV2 *vt)						asm("UFS1TO2");

#endif /* UFS_TIME_H */
