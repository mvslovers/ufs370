#include "ufs/dev.h"

__asm__("\n&FUNC    SETC 'ufs_dev_term'");
INT32 ufs_dev_term(UFSDEV *dev)
{
    if (dev->io.dvterm) {
        return dev->io.dvterm(dev);
    }
    else {
        return 0;
    }
}
