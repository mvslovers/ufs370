#include "ufs.h"
#include "ufs/file.h"

__asm__("\n&FUNC    SETC 'ufs_file_putn'");
UINT32 ufs_file_putn(UFSFILE *fp, const char *buf, UINT32 n)
{
    INT32   rc      = 0;
    UINT32  count   = 0;
    UINT32  newblk;
    UINT32  avail;
    UINT32  newsize;

    if (!fp) goto quit;
    if (!buf) goto quit;
    if (!n) goto quit;

    /* fail request if we're in an error state */
    if (fp->ind & FILE_IND_ERROR) goto quit;

    /* make sure file handle opened for write access */
    if (!(fp->mode & FILE_MODE_WRITE)) {
        fp->error = EPERM;
        fp->ind  |= FILE_IND_ERROR;
        goto quit;
    }

    /* would writing this character exceed the file size? */
    newsize = fp->filepos + n;
    if (newsize > fp->minode->dinode.filesize) {
        /* extend the file size */
        /* wtof("%s filesize=%u, newsize=%u", __func__, fp->minode->dinode.filesize, newsize); */
        rc = ufs_datablock_size_change(fp->vdisk, newsize, fp->minode);
        if (rc) {
            fp->error = rc;
            fp->ind  |= FILE_IND_ERROR;
            goto quit;
        }
        /* reset the EOF indicator */
        fp->ind &= ~FILE_IND_EOF;
    }

    /* calculate space available in this block */
    avail = fp->blksize - fp->blkpos;
    /* wtof("%s filepos=%u, blkno=%u, avail=%u", __func__, fp->filepos, fp->blkno, avail); */

    if (avail >= n) {
        /* simple copy and we're done */
        /* wtof("%s simple copy", __func__); */
        memcpy(&fp->blkbuf[fp->blkpos], buf, n);
        fp->ind |= FILE_IND_DIRTY;
        fp->filepos += n;
        fp->blkpos += n;
        count += n;
        goto quit;
    }

    /* partial copy the load new block */
    if (avail > 0) {
        /* fill the buffer */
        /* wtof("%s fill the buffer", __func__); */
        memcpy(&fp->blkbuf[fp->blkpos], buf, avail);
        fp->ind |= FILE_IND_DIRTY;
        fp->filepos += avail;
        fp->blkpos += avail;
        count += avail;
        buf += avail;
        n -= avail;
    }

    /* sync file buffer to disk */
    ufs_file_sync(fp);

    /* get next block number for this file position */
    newblk = ufs_datablock_last(fp->minode, fp->filepos+1);
    /* wtof("%s filepos=%u, newblk=%u", __func__, fp->filepos+1, newblk); */
    if (!newblk) {
        fp->error = EIO;
        fp->ind |= FILE_IND_ERROR;
        goto quit;
    }

    /* read the new block into our file handle buffer */
    fp->blkno  = newblk;
    rc = ufs_vdisk_read(fp->vdisk, fp->blkbuf, newblk);
    if (rc) {
        fp->error = rc;
        fp->ind |= FILE_IND_ERROR;
        goto quit;
    }

    /* reset the block position */
    fp->blkpos = fp->filepos % fp->blksize;
    count += ufs_file_putn(fp, buf, n);

quit:
    if (fp->ind & FILE_IND_ERROR) {
        /* make sure errno also has file error value */
        errno = fp->error;
    }
    return count;
}
