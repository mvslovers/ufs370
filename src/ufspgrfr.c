#include "ufs/pager.h"

/* free cache for disk, flush deferred write pages */
void ufs_pager_free(UFSPAGER **pager, INT32 flush)
{
    INT32       lockrc;
    UINT32      count;
    UINT32      n;
    UFSPAGER    *p;
    UFSPAGE     *page;

    if (!pager) goto quit;
    if (!*pager) goto quit;

    p = *pager;

    /* lock the pager handle so we can examine pages */
    lockrc = ufs_pager_lock(p, UFSPGRLK_TYPE_EXCL);
    if (lockrc==0) goto locked;     /* we obtained the lock */
    if (lockrc==8) goto locked;     /* we already have the lock */
    ufs_panic("%s unable to obtain exclusive lock, rc=%d", __func__, lockrc);
    return;

locked:
    if (flush) ufs_pager_flush(p);

    /* free each cache page */
    count = array_count(&p->pages);
    for(n=0; n < count; n++) {
        page = p->pages[n];

        if (!page) continue;

        ufs_page_free(&page);
        p->pages[n] = NULL;
    }

    if (p->pages) array_free(&p->pages);

    free(p);
    *pager = NULL;

unlock:
    if (lockrc==0) ufs_pager_unlock(p, UFSPGRLK_TYPE_EXCL);

quit:
    return;
}
