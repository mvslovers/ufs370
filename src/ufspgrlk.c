#include "ufs/pager.h"

/* lock pager handle */
__asm__("\n&FUNC    SETC 'ufs_pager_lock'");
INT32 ufs_pager_lock(UFSPAGER *pager, UFSPGRLK type)
{
    int     shared  = (type==UFSPGRLK_TYPE_SHARED) ? 1 : 0;
    INT32   rc      = lock(pager,shared);

    return rc;
}
