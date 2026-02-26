#include "ufs.h"
#include "ufs/file.h"

__asm__("\n&FUNC    SETC 'ufs_file_getc'");
INT32 ufs_file_getc(UFSFILE *fp)
{
    INT32   rc = 0;
    UINT32  newblk;

    /* wtof("%s enter fp=%08X", __func__, fp); */
    if (!fp) goto quit;

    if (fp->ind & FILE_IND_EOF) {
        /* wtof("%s FILE_IND_EOF", __func__); */
        rc = -1;
        goto quit;
    }

    if (fp->ind & FILE_IND_ERROR) {
        /* wtof("%s FILE_IND_ERROR", __func__); */
        rc = -1;
        goto quit;
    }

    if (!(fp->mode & FILE_MODE_READ)) {
        /* wtof("%s !FILE_MODE_READ", __func__); */
        fp->ind |= FILE_IND_ERROR;
        fp->error = EPERM;
        rc = -1;
        goto quit;
    }

    /* would reading next char exceed the file size? */
    if (fp->filepos >= fp->minode->dinode.filesize) {
        /* wtof("%s filepos=%u >= filesize=%u", __func__, fp->filepos, fp->minode->dinode.filesize); */
        fp->ind |= FILE_IND_EOF;
        rc = -1;
        goto quit;
    }

    if (fp->blkpos >= fp->blksize) {
        /* time to fetch next buffer of data */
        newblk = ufs_datablock_last(fp->minode, fp->filepos+1);
        /* wtof("%s newblk=%u", __func__, newblk); */
        if (!newblk) {
            fp->error = EIO;
            fp->ind |= FILE_IND_ERROR;
            rc = -1;
            goto quit;
        }

        /* read the new block into our file handle buffer */
        fp->blkno  = newblk;
        rc = ufs_vdisk_read(fp->vdisk, fp->blkbuf, newblk);
        if (rc) {
            fp->error = rc;
            fp->ind |= FILE_IND_ERROR;
            rc = -1;
            goto quit;
        }

        /* reset the block position */
        fp->blkpos = fp->filepos % fp->blksize;
    }

    /* get character from buffer */
    rc = fp->blkbuf[fp->blkpos++];
    fp->filepos++;

quit:
    /* wtof("%s rc=%d", __func__, rc); */
    return rc;
}
