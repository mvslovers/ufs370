#include "ufs.h"
#include "ufs/file.h"

__asm__("\n&FUNC    SETC 'ufs_fputc'");
INT32 ufs_fputc(INT32 c, UFSFILE *fp)
{
    INT32   rc = 0;
    INT32   lockrc;

    if (!fp) return -1;

    lockrc = lock(fp, 0);

    rc = ufs_file_putc(c, fp);

    if (lockrc == 0) unlock(fp, 0);

    return rc;
}
