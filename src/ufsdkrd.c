#include "ufs/disk.h"

/* read a physical block from disk dataset */
__asm__("\n&FUNC    SETC 'ufs_disk_read'");
INT32 ufs_disk_read(UFSDISK *disk, void *buf, UINT32 block)
{
    DECB    decb    = {0};
    INT32   rc;

    /* read requested block from disk dataset */
    osdread(&decb, disk->dcb, buf, disk->blksize, block);

    /* wait for I/O to complete */
    oscheck(&decb);

    /* extract return and reason codes from DECB */
    rc = (INT32) ((decb.decsdecb >> 8) & 0x0000FFFF);
    /* wtof("%s rc=%d, ecb=%08X", __func__, rc, decb.decsdecb); */

    return rc;
}
