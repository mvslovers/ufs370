#include "ufs/pager.h"

/* increment a pager counter */
__asm__("\n&FUNC    SETC 'ufs_pager_inc'");
void ufs_pager_inc(UINT32 *counter, UINT32 reset)
{
    if (counter) {
        if (*counter == 0xFFFFFFFF) {
            *counter = reset;
        }
        else {
            *counter = *counter + 1;
        }
    }
}
