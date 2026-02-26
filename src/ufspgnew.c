#include "ufs/pager.h"

/* allocate a new cache page */
__asm__("\n&FUNC    SETC 'ufs_page_new'");
UFSPAGE *ufs_page_new(unsigned blksize)
{
    UFSPAGE     *page = calloc(1, sizeof(UFSPAGE) + blksize);

    if (page) {
        page->lru   = utime64(NULL);  	/* set the time */
        page->block = (UINT32) -1;      /* make block number invalid */
    }

    return page;
}
