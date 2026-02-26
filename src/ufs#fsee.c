#include "ufs.h"
#include "ufs/file.h"

__asm__("\n&FUNC    SETC 'ufs_fseek'");
INT32 ufs_fseek(UFSFILE *fp, INT32 offset, INT32 whence)
{
    INT32       rc      = EINVAL;
    INT32       lockrc;
    UFSMIN      *minode;
    UFSDIN      *dinode;
    UINT32      newpos  = 0;
    UINT32      newblk  = 0;

    lockrc = lock(fp, 0);

    if (!fp) goto quit;

    /* setup pointers to file inode */
    minode = fp->minode;
    dinode = &minode->dinode;

    switch(whence) {
    case UFS_SEEK_SET:      /* from beginning of file   */
        if (offset >= 0)    newpos = (UINT32) offset;
        break;
    case UFS_SEEK_CUR:      /* ftom current position in file */
        if (offset < 0) {
            offset = -offset;   /* make offset a positive value */
            if ((UINT32)offset > fp->filepos) {
                /* subtract of offset from filepos would cause underflow */
                rc = EINVAL;
                goto quit;
            }
            else {
                /* should be safe to subtract offset from filepos */
                newpos = fp->filepos - offset;
            }
        }
        else {
            newpos = (UINT32) (fp->filepos + offset);
            if (newpos < fp->filepos) {
                /* overflow occured, don't change filepos */
                rc = EINVAL;
                goto quit;
            }
        }
        break;
    case UFS_SEEK_END:      /* from end of file */
        if (offset < 0) {
            offset = -offset;   /* make offset a positive value */
            if ((UINT32)offset > dinode->filesize) {
                /* subtract of offset from filesize would cause underflow */
                rc = EINVAL;
                goto quit;
            }
            else {
                /* should be safe to subtract offset from filesize */
                newpos = dinode->filesize - offset;
            }
        }
        else {
            newpos = (UINT32) (dinode->filesize + offset);
            if (newpos < dinode->filesize) {
                /* overflow occured, don't change filepos */
                rc = EINVAL;
                goto quit;
            }
        }
        break;
    }

    /* newpos is the desired file offset */
    if (fp->mode & FILE_MODE_WRITE) {
        /* file is opened for output (or append) */
        if (newpos > dinode->filesize && !(fp->mode & FILE_MODE_READ)) {
            /* we need to extend the file for this new position */
            rc = ufs_datablock_size_change(fp->vdisk, newpos, minode);
            if (rc) {
                errno = rc;
                goto quit;
            }
        }

        /* flush file handle buffer to disk */
        ufs_fsync(fp);

        if (newpos < dinode->filesize && !(fp->mode & FILE_MODE_READ)) {
            /* we need to shrink the file for this new position */
            rc = ufs_datablock_size_change(fp->vdisk, newpos, minode);
            if (rc) {
                errno = rc;
                goto quit;
            }
        }
    }

    fp->filepos = newpos;
    fp->blkpos  = fp->filepos % fp->blksize;

    /* get block number for new position */
    newblk = ufs_datablock_last(minode, newpos);
    if (!newblk) {
        /* no block allocated, zero buffer and we're done */
        memset(fp->blkbuf, 0, fp->blksize);
    }
    else if (newblk != fp->blkno) {
        /* read the new block into our file handle buffer */
        fp->blkno  = newblk;
        rc = ufs_vdisk_read(fp->vdisk, fp->blkbuf, newblk);
        if (rc) goto quit;
    }

    /* set/reset EOF indicator */
    if (fp->filepos < dinode->filesize) {
        /* reset EOF indicator */
        fp->ind &= ~FILE_IND_EOF;
    }
    else {
        /* set EOF indicator */
        fp->ind |= FILE_IND_EOF;
    }

    /* clear error indicator */
    fp->ind &= ~FILE_IND_ERROR;
    fp->error = 0;
    rc = 0;

quit:
    if (rc) {
        if (fp) {
            fp->ind |= FILE_IND_ERROR;
            fp->error = rc;
        }
        errno = rc;
    }

    if (lockrc == 0) unlock(fp, 0);
    return rc;
}
