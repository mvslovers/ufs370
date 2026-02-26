#include "ufs/disk.h"

/* write a physical block to disk dataset */
__asm__("\n&FUNC    SETC 'ufs_disk_write_n'");
INT32 ufs_disk_write_n(UFSDISK *disk, void *buf, UINT32 block, int count)
{
    INT32   rc  = 0;
	int i;

	for (i = 0; i < count; i++) {
        rc = ufs_disk_write(disk, buf, block);
		if (rc) return rc;
		buf += disk->blksize;
		block++;
	}

    return rc;
}
