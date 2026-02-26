#include "ufs/disk.h"

/* write a physical block to disk dataset */
__asm__("\n&FUNC    SETC 'ufs_disk_write'");
INT32 ufs_disk_write(UFSDISK *disk, void *buf, UINT32 block)
{
    DECB    decb    = {0};
    INT32   rc;

    if (disk->readonly) return EROFS;

    /* write requested block to disk dataset */
    osdwrite(&decb, disk->dcb, buf, disk->blksize, block);

    /* wait for I/O to complete */
    oscheck(&decb);

    /* extract return and reason codes from DECB */
    rc = (INT32) ((decb.decsdecb >> 8) & 0x0000FFFF);

    return rc;
}
