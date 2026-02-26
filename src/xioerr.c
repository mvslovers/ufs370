/* ioerr.c - ioerr */

#include <ufsi.h>

/*------------------------------------------------------------------------
 *  ioerr  -  return an error status (used for "error" entries in devtab)
 *------------------------------------------------------------------------
 */
__asm__("\n&FUNC    SETC 'ioerr'");
devcall	ioerr(void)
{
	return SYSERR;
}
