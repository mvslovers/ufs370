#include "ufs/disk.h"

/* write boot block to disk dataset */
__asm__("\n&FUNC    SETC 'ufs_disk_write_boot'");
INT32 ufs_disk_write_boot(UFSDISK *disk)
{
    char        *buf    = NULL;
    int         rc      = 0;
    UFSBOOT     *boot   = &disk->boot;
    UFSBOOTE    *bext   = disk->boote;

    if (disk->sb.readonly) {
        rc = EROFS;
        goto quit;    /* don't write a read only super block */
    }

    buf = calloc(1, disk->blksize);
    if (!buf) {
        ufs_panic("%s out of memory", __func__);
        rc = ENOMEM;
        goto quit;
    }

    /* copy boot block to buffer */
    memcpy(buf, boot, sizeof(UFSBOOT));
    if (bext) {
        time64(&bext->update_time);
        memcpy(buf+sizeof(UFSBOOT), bext, sizeof(UFSBOOTE));
    }

    /* write the boot block (block 0) */
    rc = ufs_disk_write(disk, buf, UFS_BOOTBLOCK_SECTOR);
    if (rc) goto quit;

quit:
    if (buf) free(buf);

    return rc;
}
