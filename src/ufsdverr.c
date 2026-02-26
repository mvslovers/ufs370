#include "ufs/dev.h"

__asm__("\n&FUNC    SETC 'ufs_dev_ioerr'");
INT32 ufs_dev_ioerr(UFSDEV *dev)
{
    return EIO;
}
