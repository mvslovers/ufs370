/* write.c - write */

#include <ufsi.h>

/*------------------------------------------------------------------------
 *  write  -  write one or more bytes to a device
 *------------------------------------------------------------------------
 */
__asm__("\n&FUNC    SETC 'write'");
syscall	write(
	  did32		descrp,		/* descriptor for device	*/
	  char		*buffer,	/* address of buffer		*/
	  uint32	count		/* length of buffer		*/
	)
{
	intmask		mask;		/* saved interrupt mask		*/
	struct dentry	*devptr;	/* entry in device switch table	*/
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
	retval = (*devptr->dvwrite) (devptr, buffer, count);
#if 0
	restore(mask);
#endif
	return retval;
}
