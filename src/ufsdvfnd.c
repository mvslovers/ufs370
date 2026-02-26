#include "ufs/dev.h"
#include "ufs/sys.h"

static UFSDEV *dev_find(const char *name, UFSDEV **devs);

__asm__("\n&FUNC    SETC 'ufs_dev_find'");
UFSDEV *ufs_dev_find(const char *name)
{
    UFSSYS  *sys    = ufs_sys_get();
    UFSDEV  *dev    = NULL;
    INT32   lockrc;
    UINT32  count;
    UINT32  n;

    if (!sys) return NULL;

    lockrc = lock(sys->devs, 0);
    if (lockrc!=0 && lockrc!=8) {
        ufs_panic("%s lock failure devs", __func__);
        return dev;
    }

    dev = dev_find(name, sys->devs);

    if (lockrc==0) unlock(sys->devs, 0);

    return dev;
}

__asm__("\n&FUNC    SETC 'dev_find'");
static UFSDEV *dev_find(const char *name, UFSDEV **devs)
{
    UFSDEV *dev = NULL;
    UINT32 n, count = arraycount(&devs);

    for(n=0; n < count; n++) {
        UFSDEV *d = devs[n];
        if (!d) continue;
        if (strcmp(d->name, name)==0) {
            dev = d;
            break;
        }

        if (d->children) {
            dev = dev_find(name, d->children);
            if (dev) break;
        }
    }

    return dev;
}
