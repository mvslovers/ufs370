#include "ufs/disk.h"

/* get buffer for disk I/O */
__asm__("\n&FUNC    SETC 'ufs_disk_get_buf'");
void *ufs_disk_get_buf(UFSDISK *disk)
{
    int         lockrc  = lock(disk,0);
    void        *buf    = NULL;
    UINT32      count;

    if (lockrc==0) goto locked;     /* lock acquired                        */
    if (lockrc==8) goto locked;     /* we already have a lock on the disk   */
    ufs_panic("%s lock error, rc=%d", __func__, lockrc);
    goto quit;

locked:
    count = arraycount(&disk->buf);
    if (count) buf = arraydel(&disk->buf, count);

    if (!buf) {
        /* allocate a new disk buffer */
        buf = calloc(1, disk->blksize);
        if (!buf) ufs_panic("%s out of memory", __func__);
    }

unlock:
    if (lockrc==0) unlock(disk,0);

quit:
    return buf;
}
