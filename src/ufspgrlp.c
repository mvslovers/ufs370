#include "ufs/pager.h"

/* return least recently used page from cache, or new page if below maxpages */
__asm__("\n&FUNC    SETC 'ufs_pager_lru_page'");
UFSPAGE *ufs_pager_lru_page(UFSPAGER *pager)
{
    UINT32  count   = array_count(&pager->pages);
    UFSPAGE *page   = NULL;
    UINT32  n;

    /* if we have room in cache for another page */
    if (count < pager->maxpages) goto newpage;

    /* cache is full, search for lru page */
    for(n=0; n < count; n++) {
        UFSPAGE *p = pager->pages[n];

        if (!p) continue;
        if (!page) {
            /* the first page we find is our candidate */
            page = p;
            continue;
        }

        /* check least used against previous page */
        if (p->used < page->used) {
            /* least recently used */
            page = p;
            continue;
        }

        if (p->used == page->used) {
            /* same used count, check oldest timestamp */
#if 0
            if (p->secs < page->secs) {
#else
            if (__64_cmp(&p->lru, &page->lru) == __64_SMALLER) {
#endif
                /* oldest time stamp */
                page = p;
                continue;
            }
        }
    }

    if (page) goto quit;    /* we have a least recently used page */

newpage:
    /* allocate a new page (even if maxpages is zero) */
    page = ufs_page_new(pager->disk->blksize);
    if (!page) {
        ufs_panic("%s unable to allocate new cache page", __func__);
        goto quit;
    }

    /* add page to array of cache pages */
    array_add(&pager->pages, page);

quit:
    return page;
}
