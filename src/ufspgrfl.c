#include "ufs/pager.h"

/* flush any pending writes to disk */
__asm__("\n&FUNC    SETC 'ufs_pager_flush'");
INT32 ufs_pager_flush(UFSPAGER *pager)
{
    INT32       rc      = 0;
    INT32       lockrc;
    UINT32      count;
    UINT32      n;
    UFSPAGE     *page;

    /* lock the pager handle so we can examine pages */
    lockrc = ufs_pager_lock(pager, UFSPGRLK_TYPE_EXCL);
    if (lockrc==0) goto locked;     /* we obtained the lock */
    if (lockrc==8) goto locked;     /* we already have the lock */
    ufs_panic("%s unable to obtain exclusive lock, rc=%d", __func__, lockrc);
    return lockrc;

locked:
    /* examine pages for dirty flag (pending disk write) */
    count = array_count(&pager->pages);
    for(n=0; n < count; n++) {
        page = pager->pages[n];

        if (!page) continue;
        if (page->dirty) {
            /* write this page to disk */
            rc = ufs_page_flush(pager, page);
            if (rc) break;
        }
    }

unlock:
    if (lockrc==0) ufs_pager_unlock(pager, UFSPGRLK_TYPE_EXCL);

quit:
    return rc;
}
