#include "ufs.h"
#include "ufs/file.h"

__asm__("\n&FUNC    SETC 'ufs_file_putc'");
INT32 ufs_file_putc(INT32 c, UFSFILE *fp)
{
    INT32   rc = 0;
    UINT32  newblk;

    if (!fp) return -1;

    /* fail request if we're in an error state */
    if (fp->ind & FILE_IND_ERROR) return -1;

    /* make sure file handle opened for write access */
    if (!(fp->mode & FILE_MODE_WRITE)) {
        rc = EPERM;
        goto quit;
    }

    /* would writing this character exceed the file size? */
    if (fp->filepos >= fp->minode->dinode.filesize) {
        /* extend the file size */
        rc = ufs_datablock_size_change(fp->vdisk, fp->filepos + 1, fp->minode);
        if (rc) goto quit;
    }

    /* would writing this character exceed the buffer size? */
    if (fp->blkpos >= fp->blksize) {
        /* sync file buffer to disk */
        ufs_file_sync(fp);

        /* get next block number for this file position */
        newblk = ufs_datablock_last(fp->minode, fp->filepos);
        if (!newblk) {
            rc = EIO;
            goto quit;
        }

        /* read the new block into our file handle buffer */
        fp->blkno  = newblk;
        rc = ufs_vdisk_read(fp->vdisk, fp->blkbuf, newblk);
        if (rc) goto quit;

        /* reset the block position */
        fp->blkpos = fp->filepos % fp->blksize;
    }

    /* put character into buffer */
    fp->blkbuf[fp->blkpos++] = (BYTE)c;
    fp->filepos++;
    fp->ind |= FILE_IND_DIRTY;

quit:
    if (rc) {
        fp->ind |= FILE_IND_ERROR;
        fp->error = rc;
        rc = -1;
    }
    return rc;
}
