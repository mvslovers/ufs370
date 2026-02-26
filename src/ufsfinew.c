#include "ufs/file.h"

__asm__("\n&FUNC    SETC 'ufs_file_new'");
UFSFILE *ufs_file_new(UFSSYS *sys, ACEE *acee, UFSMIN *minode)
{
    UFSFILE     *file   = NULL;
    INT32       lockrc  = -1;

    if (!sys) {
        ufs_panic("%s missing required parm sys", __func__);
        goto quit;
    }

    /* allocate file handle */
    file = calloc(1, sizeof(UFSFILE));
    if (!file) {
        ufs_panic("%s out of memory", __func__);
        goto quit;
    }

    /* initialize file handle */
    strcpy(file->eye, UFS_FILE_EYE);
    file->sys       = sys;
    file->acee      = acee;
    file->minode    = minode;

    if (minode) {
        file->vdisk = minode->vdisk;
        file->blksize = file->vdisk->disk->blksize;

        /* allocate a disk buffer */
        file->blkbuf = ufs_disk_get_buf(file->vdisk->disk);
        if (!file->blkbuf) {
            ufs_panic("%s unable to get disk buffer", __func__);
            ufs_file_free(&file);
            goto quit;
        }
    }

    /* add file handle to system array of file handles */
    lockrc = lock(sys, 0);
    array_add(&sys->files, file);
    if (lockrc==0) unlock(sys, 0);

quit:
    return file;
}
