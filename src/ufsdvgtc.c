#include "ufs/dev.h"

__asm__("\n&FUNC    SETC 'ufs_dev_getc'");
INT32 ufs_dev_getc(UFSDEV *dev)
{
    if (dev->io.dvgetc) {
        return dev->io.dvgetc(dev);
    }
    else {
        return -1;
    }
}
