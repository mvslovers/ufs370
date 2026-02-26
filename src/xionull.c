/* ionull.c - ionull */
#include <ufsi.h>

/*------------------------------------------------------------------------
 *  ionull  -  do nothing (used for "don't care" entries in devtab)
 *------------------------------------------------------------------------
 */
__asm__("\n&FUNC    SETC 'ionull'");
devcall	ionull(void)
{
	return OK;
}
