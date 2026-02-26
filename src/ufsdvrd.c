#include "ufs/dev.h"

__asm__("\n&FUNC    SETC 'ufs_dev_read'");
INT32 ufs_dev_read(UFSDEV *dev, void *buf, UINT32 sz)
{
    if (dev->io.dvread) {
        return dev->io.dvread(dev, buf, sz);
    }
    else {
        return EPERM;
    }
}
