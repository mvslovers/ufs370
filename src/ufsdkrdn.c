#include "ufs/disk.h"

/* read 1 or more physical blocks from disk dataset */
__asm__("\n&FUNC    SETC 'ufs_disk_read_n'");
INT32 ufs_disk_read_n(UFSDISK *disk, void *buf, UINT32 block, int count)
{
    INT32   rc = 0;
	int i;

	for (i = 0; i < count; i++) {
        rc = ufs_disk_read(disk, buf, block);
		if (rc) return rc;
		buf += disk->blksize;
		block++;
	}

	return rc;
}
