#include "ufs/pager.h"

/* create new page cache for disk */
__asm__("\n&FUNC    SETC 'ufs_pager_new'");
UFSPAGER *ufs_pager_new(UFSDISK *disk, UINT32 maxpages)
{
    UFSPAGER *pager = calloc(1, sizeof(UFSPAGER));

    if (pager) {
        strcpy(pager->eye, UFSPAGER_EYE);
        pager->disk     = disk;
        pager->maxpages = maxpages;
    }

    return pager;
}
