/* close.c - close */

#include <ufsi.h>

/*------------------------------------------------------------------------
 *  close  -  close a device
 *------------------------------------------------------------------------
 */
__asm__("\n&FUNC    SETC 'close'");
syscall	close(
	  did32		descrp		/* descriptor for device	*/
	)
{
	intmask		mask;		/* saved interrupt mask		*/
	struct	dentry	*devptr;	/* entry in device switch table	*/
	int32		retval;		/* value to return to caller	*/
#if 0
	mask = disable();
#endif
	if (isbaddev(descrp)) {
#if 0
		restore(mask);
#endif
		return SYSERR;
	}

	devptr = (struct dentry *) &devtab[descrp];
	retval = (*devptr->dvclose) (devptr);
#if 0
	restore(mask);
#endif
	return retval;
}
