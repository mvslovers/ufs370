#include "ufs/dev.h"
#include "ufs/sys.h"

__asm__("\n&FUNC    SETC 'ufs_dev_new'");
UFSDEV *ufs_dev_new(INT32 num, INT32 minor, const char *name,
                    void *vsys, UFSDEV *parent,
                    void *ctx1, void *ctx2, const UFSDEVIO *devio)
{
    UFSSYS  *sys    = (UFSSYS*)vsys;
    UFSDEV  *dev;
    INT32   lockrc;
    void    *lockword;
    char    tmp[UFSDEV_MAX_NAME+1];

    if (!sys) sys = ufs_sys_get();

    lockword = sys->devs;       /* we do this because sys->devs could change by arrayadd() */
    lockrc = lock(lockword,0);
    if (lockrc!=0 && lockrc!=8) {
        ufs_panic("%s lock failure", __func__);
        return NULL;
    }

    dev = calloc(1,sizeof(UFSDEV));
    for(;dev;) {
        strcpy(dev->eye, UFSDEV_EYE);

        sys->next_dvnum++;
        if (!num) num = sys->next_dvnum;

        if (!name) {
            snprintf(tmp, UFSDEV_MAX_NAME, "dev%04u", num);
            tmp[UFSDEV_MAX_NAME] = 0;
            name = tmp;
        }

        dev->num        = num;
        dev->minor      = minor;
        strcpyp(dev->name, sizeof(dev->name), (void*)name, 0);
        dev->sys        = sys;
        dev->parent     = parent;
        dev->ctx1       = ctx1;     /* device dependent context */
        dev->ctx2       = ctx2;     /* device dependent context */
        if (devio) dev->io = *devio;   /* structure copy */

        /* request the device initialize itself */
        if (ufs_dev_init(dev)) {
            /* initialization failed, free this device */
            ufs_panic("%s device initialization failed for \"%s\"", __func__, dev->name);
            ufs_dev_free(&dev);
            break;
        }

        /* device init okay (or no init needed) */
        if (parent) {
            /* make this dev a child of parent */
            arrayadd(&parent->children, dev);
        }
        else {
            /* must be a base device */
            /* add to system array of devices */
            arrayadd(&sys->devs, dev);
        }
        break;
    }

    if (lockrc==0) unlock(lockword,0);
    return dev;
}
