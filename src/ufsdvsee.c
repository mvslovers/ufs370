#include "ufs/dev.h"

__asm__("\n&FUNC    SETC 'ufs_dev_seek'");
INT32 ufs_dev_seek(UFSDEV *dev, INT32 whence)
{
    if (dev->io.dvseek) {
        return dev->io.dvseek(dev,whence);
    }
    else {
        return EPERM;
    }
}
