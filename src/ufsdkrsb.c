#include "ufs/disk.h"

/* read super block from disk dataset */
__asm__("\n&FUNC    SETC 'ufs_disk_read_super'");
INT32 ufs_disk_read_super(UFSDISK *disk, UFSSB *sb)
{
    char    *buf    = calloc(1, disk->blksize);
    int     rc;

    if (!buf) {
        ufs_panic("%s out of memory", __func__);
        rc = ENOMEM;
        goto quit;
    }

    /* read the super block (block 1) */
    rc = ufs_disk_read(disk, buf, UFS_SUPERBLOCK_SECTOR);
    if (rc) goto quit;

    /* success, return super block to caller */
    memcpy(sb, buf, sizeof(UFSSB));
    sb->modified    = 0;                /* reset just in case it was saved */

quit:
    if (buf) free(buf);
    return rc;
}
