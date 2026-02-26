#include "ufs.h"
#include "ufs/file.h"

__asm__("\n&FUNC    SETC 'ufs_fgetc'");
INT32 ufs_fgetc(UFSFILE *fp)
{
    INT32   rc = 0;
    INT32   lockrc;

    if (!fp) return rc;

    /* lock file handle */
    lockrc = lock(fp, 0);

    /* read one character from file handle */
    rc = ufs_file_getc(fp);

    /* unlock file handle */
    if (lockrc == 0) unlock(fp, 0);

    return rc;
}
