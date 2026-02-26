#include "ufs/dev.h"

__asm__("\n&FUNC    SETC 'ufs_dev_write'");
INT32 ufs_dev_write(UFSDEV *dev, void *buf, UINT32 sz)
{
    if (dev->io.dvwrite) {
        return dev->io.dvwrite(dev, buf, sz);
    }
    else {
        return EPERM;
    }
}
