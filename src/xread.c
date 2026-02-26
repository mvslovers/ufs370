/* read.c - read */

#include <ufsi.h>

/*------------------------------------------------------------------------
 *  read  -  read one or more bytes from a device
 *------------------------------------------------------------------------
 */
__asm__("\n&FUNC    SETC 'read'");
syscall	read(
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
	retval = (*devptr->dvread) (devptr, buffer, count);
#if 0
	restore(mask);
#endif
	return retval;
}
