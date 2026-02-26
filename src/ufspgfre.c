#include "ufs/pager.h"

/* deallocate a cache page */
__asm__("\n&FUNC    SETC 'ufs_page_free'");
void ufs_page_free(UFSPAGE **page)
{
    if (page && *page) {
        free(*page);
        *page = NULL;
    }
}
