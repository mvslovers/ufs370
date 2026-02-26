#include "ufs/disk.h"

/* write super block to disk dataset */
__asm__("\n&FUNC    SETC 'ufs_disk_write_super'");
INT32 ufs_disk_write_super(UFSDISK *disk, UFSSB *sb)
{
    char    *buf    = NULL;
    int     rc      = 0;
    UFSSB   *sbu;

    if (sb->readonly) {
        rc = EROFS;
        goto quit;    /* don't write a read only super block */
    }

    buf = calloc(1, disk->blksize);
    if (!buf) {
        ufs_panic("%s out of memory", __func__);
        rc = ENOMEM;
        goto quit;
    }

    /* copy super block to buffer */
    memcpy(buf, sb, sizeof(UFSSB));

    /* reset/update some of the super block fields */
    sbu = (UFSSB*)buf;
    sbu->modified    = 0;            /* reset the modified flag  */
#if 1
    if (!disk->boote || disk->boote->version == UFS_DISK_BOOT_0)
#endif
    sbu->update_time = time(NULL);   /* set the update time      */

    /* write the super block (block 1) */
    rc = ufs_disk_write(disk, buf, UFS_SUPERBLOCK_SECTOR);
    if (rc) goto quit;

    /* successful, copy updated super block back to caller */
    memcpy(sb, sbu, sizeof(UFSSB));

#if 1
    /* update the boot extension in the boot block #0 */
    if (disk->boote && disk->boote->version > UFS_DISK_BOOT_0) {
        rc = ufs_disk_write_boot(disk);
    }
#endif

quit:
    if (buf) free(buf);

    return rc;
}
