#include "ufs.h"
#include "ufs/pager.h"

__asm__("\n&FUNC    SETC 'ufs_sync'");
void ufs_sync(UFS *ufs)
{
    UFSSYS      *sys    = ufs ? ufs->sys : ufs_sys_get();
    UFSSB       *sb;
    INT32       lockrc;
    INT32       lockdisk;
    UFSDISK     *disk;
    UFSPAGER    *pager;
    UINT32      count;
    UINT32      n;

    if (!sys) return;

    lockrc = lock(sys, 0);
    if (lockrc==0) goto locked;     /* we obtained the lock */
    if (lockrc==8) goto locked;     /* we already have the lock */
    ufs_panic("%s unable to obtain exclusive lock, rc=%d", __func__, lockrc);
    return;

locked:
    if (sys->pagers) {
        count = array_count(&sys->pagers);
        for(n=0; n < count; n++) {
            pager = sys->pagers[n];
            if (!pager) continue;

            disk = pager->disk;
            lockdisk = lock(disk, 0);

            /* see if the super block needs to be saved */
            if (disk->sb.modified) {
                if (ufs_disk_write_super(disk, &disk->sb)) {
                    wtof("%s write superblock failed", __func__);
                }
                else {
                    disk->sb.modified = 0;
                }
            }
            /* flush any cached pages that haven't been saved */
            ufs_pager_flush(pager);

            if (lockdisk==0) unlock(disk, 0);
        }
    }

    if (lockrc==0) unlock(sys, 0);
}
