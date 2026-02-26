#include "ufs/dev.h"

__asm__("\n&FUNC    SETC 'ufs_dev_free'");
void ufs_dev_free(UFSDEV **dev)
{
    UINT32  count;
    UINT32  n;

    if (dev && *dev) {
        UFSDEV *d = *dev;

        /* request the device terminate itself */
        ufs_dev_term(d);

        /* cleanup the children */
        if (d->children) {
            count = arraycount(&d->children);

            for(n=0; n < count; n++) {
                ufs_dev_free(&(d->children[n]));
            }

            arrayfree(&d->children);
        }

        /* release the device handle */
        free(*dev);
        *dev = NULL;
    }
}
