#include "ufs/pager.h"

/* write one or more blocks to cache and disk dataset */
__asm__("\n&FUNC    SETC 'ufs_pager_write_n'");
INT32 ufs_pager_write_n(UFSPAGER *pager, void *buf, UINT32 block, int count)
{
    INT32   rc      = 0;
	int i;

	for (i = 0; i < count; i++) {
        rc = ufs_pager_write(pager, buf, block);
		if (rc) return rc;
		buf += pager->disk->blksize;
		block++;
	}

    return rc;
}
