#include "ufs/disk.h"

/* free disk I/O buffer */
__asm__("\n&FUNC    SETC 'ufs_disk_free_buf'");
void ufs_disk_free_buf(UFSDISK *disk, void *buf)
{
    int         lockrc  = lock(disk,0);

    if (lockrc==0) goto locked;     /* lock acquired                        */
    if (lockrc==8) goto locked;     /* we already have a lock on the disk   */
    ufs_panic("%s lock failed, rc=%d", __func__, lockrc);
    return;

locked:
    if (buf) arrayadd(&disk->buf, buf);

unlock:
    if (lockrc==0) unlock(disk,0);

    return;
}
