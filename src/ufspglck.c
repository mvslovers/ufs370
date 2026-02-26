#include "ufs/pager.h"

/* lock page */
__asm__("\n&FUNC    SETC 'ufs_page_lock'");
INT32 ufs_page_lock(UFSPAGE *page, UFSPGRLK type)
{
    int     shared  = (type==UFSPGRLK_TYPE_SHARED) ? 1 : 0;
    INT32   rc      = lock(page,shared);

    return rc;
}
