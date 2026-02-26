#include "ufs/pager.h"

/* find a block in the cache */
__asm__("\n&FUNC    SETC 'ufs_pager_find_block'");
UFSPAGE *ufs_pager_find_block(UFSPAGER *pager, UINT32 block)
{
    UINT32      count   = array_count(&pager->pages);
    UFSPAGE     *page;
    UINT32      n;

    for(n=0; n < count; n++) {
        page = pager->pages[n];

        if (!page) continue;
        if (page->block == block) {
            return page;
        }
    }

    return NULL;
}
