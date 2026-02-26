#include "ufs/dev.h"

__asm__("\n&FUNC    SETC 'ufs_dev_close'");
INT32 ufs_dev_close(UFSDEV *dev)
{
    if (dev->io.dvclose) {
        return dev->io.dvclose(dev);
    }
    else {
        return EPERM;
    }
}
