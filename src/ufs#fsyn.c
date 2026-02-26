#include "ufs.h"
#include "ufs/file.h"

__asm__("\n&FUNC    SETC 'ufs_fsync'");
void ufs_fsync(UFSFILE *fp)
{
    INT32   lockrc;

    if (!fp) return;

    lockrc = lock(fp, 0);

    ufs_file_sync(fp);

    if (lockrc == 0) unlock(fp, 0);
}
