#include "ufs/pager.h"

/* unlock page */
__asm__("\n&FUNC    SETC 'ufs_page_unlock'");
INT32 ufs_page_unlock(UFSPAGE *page, UFSPGRLK type)
{
    int     shared  = (type==UFSPGRLK_TYPE_SHARED) ? 1 : 0;
    INT32   rc      = unlock(page,shared);

    return rc;
}
