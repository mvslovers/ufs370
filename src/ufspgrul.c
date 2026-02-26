#include "ufs/pager.h"

/* unlock pager handle */
__asm__("\n&FUNC    SETC 'ufs_pager_unlock'");
INT32 ufs_pager_unlock(UFSPAGER *pager, UFSPGRLK type)
{
    int     shared  = (type==UFSPGRLK_TYPE_SHARED) ? 1 : 0;
    INT32   rc      = unlock(pager,shared);

    return rc;
}
