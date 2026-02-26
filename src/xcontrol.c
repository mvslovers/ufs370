/* control.c - control */

#include <ufsi.h>

/*------------------------------------------------------------------------
 *  control  -  control a device or a driver (e.g., set the driver mode)
 *------------------------------------------------------------------------
 */
__asm__("\n&FUNC    SETC 'control'");
syscall	control(
	  did32		descrp,		/* descriptor for device	*/
	  int32		func,		/* specific control function	*/
	  int32		arg1,		/* specific argument for func	*/
	  int32		arg2		/* specific argument for func	*/
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
	retval = (*devptr->dvcntl) (devptr, func, arg1, arg2);
#if 0
	restore(mask);
#endif
	return retval;
}
