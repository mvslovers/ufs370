#include "ufs/time.h"

/* deprecated */

/* get GMT time as timeval value with Unix epoch */
__asm__("\n&FUNC    SETC 'ufs_timeval'");
UFSTIMEV ufs_timeval(UFSTIMEV *tv)
{
    unsigned long long  tod;
	UFSTIMEV1			*tv1 = (UFSTIMEV1*)tv;
    UFSTIMEV1           tmp;

    if (!tv1) tv1 = &tmp;

    /* get STCK value */
    __asm__("STCK\t0(%0)" : : "r" (&tod));

    /* make Jan 1 1900 (STCK) relative to Jan 1 1970 (unix epoch) */
    tod -=  0x7D91048BCA000000ULL;  /* STCK value for Jan 1 1970 */

    /* convert to microseconds (bits 0-51==number of microseconds) */
    tod >>= 12; /* convert to microseconds (1 us = .000001 sec) */

    /* calc seconds and microseconds (divide by 1000000) */
    __asm__(
        "LM\t0,1,0(%0)       load TOD microseconds\n\t"
        "D\t0,=F'1000000'  divide by 1000000\n\t"
        "ST\t1,0(0,%1)       store seconds (quotient)\n\t"
        "ST\t0,4(0,%1)       store microseconds (remainder)"
        : : "r" (&tod), "r" (tv1));

quit:
    return *(UFSTIMEV*)(tv1);
}
