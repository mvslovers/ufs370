#include "ufs.h"
#include "ufs/file.h"

__asm__("\n&FUNC    SETC 'ufs_file_sync'");
void ufs_file_sync(UFSFILE *fp)
{
    if (!fp) return;

    if (fp->vdisk) {
        /* cleanup disk based resources */
        if (fp->ind & FILE_IND_DIRTY) {
            if (fp->minode && !fp->blkno) {
                /* get block number for the current position */
                fp->blkno = ufs_datablock_last(fp->minode, fp->filepos);
            }

            if (fp->blkbuf && fp->blkno) {
                /* rewrite the block buffer */
                ufs_vdisk_write(fp->vdisk, fp->blkbuf, fp->blkno);
                fp->ind &= ~FILE_IND_DIRTY;
            }
        }
    }

    if (fp->minode) {
        /* cleanup inode based resources */
        if (fp->minode->flags & MINODE_FLAG_DIRTY) {
            ufs_inode_writeback(fp->minode);
            fp->minode->flags &= ~MINODE_FLAG_DIRTY;
        }
    }
}
