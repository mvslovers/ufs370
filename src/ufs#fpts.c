#include "ufs.h"
#include "ufs/file.h"

__asm__("\n&FUNC    SETC 'ufs_fputs'");
INT32 ufs_fputs(const char *str, UFSFILE *fp)
{
    INT32   rc;
    INT32   lockrc;
    UINT32  n;
    UINT32  num;

    if (!str || !fp) {
        rc = UFS_EOF;
        goto quit;
    }

    rc = 0;

    /* lock file handle */
    lockrc = lock(fp, 0);

    /* write string to file excluding 0 byte */
    num = (UINT32) strlen(str);
    if (num > 0) {
        n = ufs_file_putn(fp, str, num);
        if (n != num) rc = UFS_EOF;
    }

    /* unlock file handle */
    if (lockrc == 0) unlock(fp, 0);

quit:
    return rc;
}
