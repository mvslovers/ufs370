#include "ufs/dev.h"

__asm__("\n&FUNC    SETC 'ufs_dev_cntl'");
INT32 ufs_dev_cntl(UFSDEV *dev, INT32 a, INT32 b, INT32 c)
{
    if (dev->io.dvcntl) {
        return dev->io.dvcntl(dev,a,b,c);
    }
    else {
        return EPERM;
    }
}
