#include "ufs/dev.h"

__asm__("\n&FUNC    SETC 'ufs_dev_null'");
INT32 ufs_dev_null(UFSDEV *dev)
{
    return 0;
}
