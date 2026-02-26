#include "ufs/name.h"
#include "ufs/sys.h"

/* TODO: this function need to be replaced, we're not doing DEV's */
__asm__("\n&FUNC    SETC 'ufs_name_init'");
INT32 ufs_name_init(UFSDEV *dev)
{
    INT32   error       = 0;
    UFSDEV  *lfiledev   = NULL;
    UFSDEV  *rfiledev   = NULL;
    UFSSYS  *sys;
    INT32   lockrc;
    UINT32  n;
    UINT32  count;
    INT32   i;
	char	tmpstr[256];

	strcpy(dev->name, NM_NAME);         /* make sure the name space device has this name */

    if (!dev->ctx1) {
        dev->ctx1 = arraynew(20);       /* ctx1 is our dynamic array of UFSNAME handles */
        if (!dev->ctx1) return ENOMEM;  /* well that sucks! */
    }

    /* create "/dev/devicename" entries in the name table */
    sys = (UFSSYS *)dev->sys;

    lockrc = lock(sys,0);
    if (lockrc!=0 && lockrc!=8) {
        ufs_panic("%s lock failure sys", __func__);
        error = EPERM;
        goto quit;
    }

    count = arraycount(&sys->devs);
    for(n=0; n < count; n++) {
        UFSDEV *d = sys->devs[n];

        if (!d) continue;               /* no device handle     */
        if (!d->name[0]) continue;      /* device has no name   */

        snprintf(tmpstr, sizeof(tmpstr), "/dev/%s", dev->name);
        tmpstr[sizeof(tmpstr)-1] = 0;
        for(i=0; tmpstr[i];i++) {
            tmpstr[i] = tolower(tmpstr[i]);
        }
#if 0
        error = ufs_name_mount(tmpstr, "", d);
        if (error) break;
#endif
    }

quit:
    if (lockrc==0) unlock(sys,0);

    return error;
}
