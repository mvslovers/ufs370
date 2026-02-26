#include "ufs.h"
#include "ufs/file.h"

__asm__("\n&FUNC    SETC 'ufs_file_getn'");
UINT32 ufs_file_getn(UFSFILE *fp, char *buf, UINT32 n)
{
    INT32   rc      = 0;
    UINT32  count   = 0;
    UINT32  newblk;
    UINT32  favail;
    UINT32  avail;

    if (!fp) goto quit;
    if (!buf) goto quit;
    if (!n) goto quit;

    if (fp->ind & FILE_IND_EOF) goto quit;
    if (fp->ind & FILE_IND_ERROR) goto quit;

    if (!(fp->mode & FILE_MODE_READ)) {
        fp->ind |= FILE_IND_ERROR;
        fp->error = EPERM;
        goto quit;
    }

    /* wtof("%s filepos=%u, blkno=%u", __func__, fp->filepos, fp->blkno); */

    /* check for EOF */
    if (fp->filepos >= fp->minode->dinode.filesize) {
        /* wtof("%s 1,filepos=%u, filesize=%u, EOF", __func__, fp->filepos, fp->minode->dinode.filesize); */
        fp->ind |= FILE_IND_EOF;
        goto quit;
    }

    /* calc number of bytes remaining in file */
    favail = fp->minode->dinode.filesize - fp->filepos;
    /* if (favail >= n) { */
    if (favail > 0) {
        /* calc number of bytes remaining in buffer */
        avail = fp->blksize - fp->blkpos;

        /* use smaller value, file or buffer available byte count */
        if (avail > favail) avail = favail;

        /* can we complete the request using avail byte count? */
        if (avail >= n) {
            /* we have enough bytes in buffer to complete the request */
            memcpy(buf, &fp->blkbuf[fp->blkpos], n);
            fp->blkpos += n;
            fp->filepos += n;
            count = n;
            goto quit;
        }

        /* we'll copy avail bytes and then load the next buffer */
        if (avail > 0) {
            /* copy the available bytes in the file handle buffer */
            memcpy(buf, &fp->blkbuf[fp->blkpos], avail);
            buf += avail;
            fp->blkpos += avail;
            fp->filepos += avail;
            count += avail;
            n -= avail;
        }

        /* check for EOF */
        if (fp->filepos >= fp->minode->dinode.filesize) {
            /* wtof("%s 2, filepos=%u, filesize=%u, EOF", __func__, fp->filepos, fp->minode->dinode.filesize); */
            fp->ind |= FILE_IND_EOF;
            goto quit;
        }

        /* time to fetch next buffer of data */
        newblk = ufs_datablock_last(fp->minode, fp->filepos+1);
        /* wtof("%s newblk=%u", __func__, newblk); */
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

        /* recursive call to get remaining bytes */
        count += ufs_file_getn(fp, buf, n);
        goto quit;
    }

    /* if we're here then the requested bytes will likely cause EOF or Error
    ** so we'll use the slower ufs_file_getc() calls to retrieve what we can
    ** one byte at a time until EOF or Error occurs.
    */
    /* wtof("%s doing ufs_file_getc() %u times", __func__, n); */
    while(n > 0) {
        rc = ufs_file_getc(fp);
        if (rc < 0) goto quit;
        *buf++ = rc;
        count++;
        n--;
    }

quit:
    if (fp->ind & FILE_IND_ERROR) {
        /* make sure errno also has file error value */
        errno = fp->error;
    }
    return count;
}
