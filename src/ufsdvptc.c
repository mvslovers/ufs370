#include "ufs/dev.h"

__asm__("\n&FUNC    SETC 'ufs_dev_putc'");
INT32 ufs_dev_putc(UFSDEV *dev, INT32 c)
{
    if (dev->io.dvputc) {
        return dev->io.dvputc(dev,c);
    }
    else {
        return EPERM;
    }
}
