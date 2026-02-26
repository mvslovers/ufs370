#include "ufs/pager.h"

/* flush a cache page */
__asm__("\n&FUNC    SETC 'ufs_page_flush'");
INT32 ufs_page_flush(UFSPAGER *pager, UFSPAGE *page)
{
    INT32   rc;

    /* wtodumpf(page, sizeof(UFSPAGE), "%s PAGE", __func__); */
    rc  = ufs_disk_write(pager->disk, page->buf, page->block);
    if (rc) {
        wtof("%s page->block=%u", __func__, page->block);
        wtodumpf(page, sizeof(UFSPAGE), "%s PAGE", __func__);
        wtodumpf(pager, sizeof(UFSPAGER), "PAGER");
        wtodumpf(pager->disk, sizeof(UFSDISK), "PAGER->DISK");
        ufs_panic("ufs_page_flush() unable to flush cache data to disk, rc=%d", rc);
        goto quit;
    }

    /* success, increment physical disk write counter */
    ufs_pager_inc(&pager->dwrites, 1);

    /* reset the dirty flag */
    page->dirty = 0;

quit:
    return rc;
}
