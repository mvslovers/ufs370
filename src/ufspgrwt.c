#include "ufs/pager.h"

/* write block to cache or disk */
__asm__("\n&FUNC    SETC 'ufs_pager_write'");
INT32 ufs_pager_write(UFSPAGER *pager, void *buf, UINT32 block)
{
    INT32       rc      = -1;
    INT32       lockrc  = 0;
    UFSDISK     *disk   = pager->disk;
    UFSPAGE     *page;

    lockrc = ufs_pager_lock(pager, UFSPGRLK_TYPE_EXCL);
    if (lockrc==0) goto locked;     /* we obtained the lock */
    if (lockrc==8) goto locked;     /* we already have the lock */
    ufs_panic("%s unable to obtain exclusive lock, rc=%d", __func__, lockrc);
    return lockrc;

locked:
    ufs_pager_inc(&pager->writes, 0);

    /* if a block is in the cache we want to defer writes */
    page = ufs_pager_find_block(pager, block);
    if (page) {
        ufs_pager_inc(&pager->cachehits, 1);
        ufs_pager_inc(&page->used, 1);

        page->lru = utime64(NULL);
        memcpy(page->buf, buf, disk->blksize);
        page->dirty++;
        rc = 0;
        goto unlock;
    }

    /* block was not found in cache for this disk */
    /* write requested block to disk dataset */
    rc = ufs_disk_write(disk, buf, block);
    if (rc) {
        ufs_panic("%s write to disk dataset failed, rc=%d", __func__, rc);
        goto unlock;
    }

    /* increment physical disk writes counter */
    ufs_pager_inc(&pager->dwrites, 1);

    /* locate least recently used page in cache,
       (may allocate a new page if below maxpages */
    page = ufs_pager_lru_page(pager);
    if (!page) {
        ufs_panic("%s no page returned for ufs_pager_lru_page()", __func__);
        goto unlock;
    }

    /* we have a lru page, flush page if dirty */
    if (page->dirty) {
        rc = ufs_page_flush(pager,page);
        if (rc) goto unlock;
    }

    /* reset page with new cache data */
    page->lru   = utime64(NULL);
    page->dirty = 0;
    page->used  = 1;
    page->block = block;
    memcpy(page->buf, buf, disk->blksize);

unlock:
    if (lockrc==0) ufs_pager_unlock(pager, UFSPGRLK_TYPE_EXCL);

quit:
    return rc;
}
