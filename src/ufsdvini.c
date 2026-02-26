#include "ufs/dev.h"

__asm__("\n&FUNC    SETC 'ufs_dev_init'");
INT32 ufs_dev_init(UFSDEV *dev)
{
    if (dev->io.dvinit) {
        return dev->io.dvinit(dev);
    }
    else {
        /* dvinit is optional */
        return 0;
    }
}
