#include "ufs/inode.h"

__asm__("\n&FUNC    SETC 'ufs_inode_use'");
UFSMIN *ufs_inode_use(UFSMIN *minode)
{
    INT32   lockrc;

    lockrc = lock(minode,0);

    /* bump inode use count */
    minode->usecount++;   /* increment use count */

    if (lockrc==0)    unlock(minode,0);

    return minode;
}
