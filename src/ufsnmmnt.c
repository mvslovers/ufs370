#include "ufs/name.h"
#include "ufs/sys.h"

__asm__("\n&FUNC    SETC 'ufs_name_mount'");
INT32 ufs_name_mount(const char *prefix, const char *rep, UFSVDISK *vdisk)
{
    INT32   error       = 0;
    UFSSYS  *sys        = ufs_sys_get();
    INT32   lockrc;
    UFSNAME *name;

    if (!sys) return EPERM;

    lockrc = lock(sys,0);
    if (lockrc!=0 && lockrc!=8) {
        ufs_panic("%s lock failure", __func__);
        error = EPERM;
        goto quit;
    }

    name = ufs_name_find(prefix);
    if (!name) {
        name = calloc(1, sizeof(UFSNAME));
        if (!name) {
            ufs_panic("%s out of memory", __func__);
            error = ENOMEM;
            goto quit;
        }
        arrayadd(&sys->names, name);
    }

    if (name->prefix)   free(name->prefix);
    name->prefix = strdup(prefix);

    if (name->rep)      free(name->rep);
    name->rep = strdup(rep);

    name->vdisk = vdisk;

    ufs_name_sort();

quit:
    if (lockrc==0) unlock(sys,0);
    return error;
}
