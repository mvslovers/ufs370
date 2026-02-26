#include "ufs/file.h"

__asm__("\n&FUNC    SETC 'ufs_file_free'");
void ufs_file_free(UFSFILE **file)
{
    UFSFILE *f;
    UINT32  count;
    UINT32  n;
    UFSSYS  *sys;

    if (file && *file) {
        f = *file;
        sys = f->sys;

        if (sys) {
            /* remove the file handle from the system array of file handles */
            INT32 lockrc = lock(sys,0);
            count = array_count(&sys->files);
            for(n=count; n > 0; n--) {
                UFSFILE *sf = array_get(&sys->files, n);
                if (!sf) continue;
                if (sf != f) continue;
                /* found it, remove from array */
                array_del(&sys->files, n);
                break;
            }
            if (lockrc==0) unlock(sys,0);
        }

        if (f->vdisk && f->blkbuf) {
            ufs_disk_free_buf(f->vdisk->disk, f->blkbuf);
            f->blkbuf = NULL;
        }

        if (f->minode) {
            /* release the inode */
            ufs_inode_rel(f->minode);
            f->minode = NULL;
        }

        /* make sure we've cleaned up internal resources */
        if (f->blkbuf) {
            ufs_panic("%s file handle buffer not freed", __func__);
        }

        free(f);
        *file = NULL;
    }
}
