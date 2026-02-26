/* putc.c - putc */

#include <ufsi.h>

/*------------------------------------------------------------------------
 *  putc  -  send one character of data (byte) to a device
 *------------------------------------------------------------------------
 */
__asm__("\n&FUNC    SETC 'putc'");
syscall	putc(
	  did32		descrp,		/* descriptor for device	*/
	  char		ch		/* character to send		*/
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
	retval = (*devptr->dvputc) (devptr, ch);
#if 0
	restore(mask);
#endif
	return retval;
}
