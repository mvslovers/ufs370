#include "ufs.h"
#include "ufs/file.h"

__asm__("\n&FUNC    SETC 'ufs_fopen'");
UFSFILE *ufs_fopen(UFS *ufs, const char *path, const char *mode)
{
    UFSFILE     *fp         = NULL;
    INT32       rc          = 0;
    BYTE        fm          = ufs_file_mode(mode);

    /* check for valid open mode (must have at least read or write) */
    if (!(fm & (FILE_MODE_READ + FILE_MODE_WRITE))) {
        rc = EINVAL;
        goto quit;
    }

    /* open file if it exist */
    rc = ufs_open(ufs, path, fm, &fp);
    if (rc==ENOENT && fm & FILE_MODE_CREATE) {
        /* create file */
        rc = ufs_create(ufs, path, fm, &fp);
    }

quit:
    errno = rc;
    return fp;
}
