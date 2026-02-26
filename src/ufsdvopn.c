#include "ufs/dev.h"

__asm__("\n&FUNC    SETC 'ufs_dev_open'");
INT32 ufs_dev_open(UFSDEV *dev, char *filename, char *mode)
{
    if (dev->io.dvopen) {
        return dev->io.dvopen(dev, filename, mode);
    }
    else {
        return EPERM;
    }
}
