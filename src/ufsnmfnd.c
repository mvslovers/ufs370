#include "ufs/name.h"
#include "ufs/sys.h"

__asm__("\n&FUNC    SETC 'ufs_name_find'");
UFSNAME *ufs_name_find(const char *name)
{
    UFSSYS  *sys        = ufs_sys_get();
    UFSNAME *found      = NULL;
    UFSNAME **names     = NULL;
    INT32   lockrc      = -1;
    UINT32  len         = name ? strlen(name) : 0;
    UINT32  count;
    UINT32  n;
    char    tmpname[NM_MAXLEN];

    if (!sys) return NULL;

    if (!name)  name = "";
    if (len >= NM_MAXLEN) return NULL;

    lockrc = lock(sys, 0);
    if (lockrc!=0 && lockrc!=8) {
        ufs_panic("%s lock failure namedev", __func__);
        return NULL;
    }

    names = sys->names;
    count = arraycount(&names);

    /* repeatedly substitute the name prefix until a non-namespace */
    /* device is reached or an iteration limit is exceeded */
    for (n=0; n<count ; n++) {
        UFSNAME *p = names[n];

        if (!p) continue;

        if (strcmp(p->prefix, name)==0) {
            found = p;
            break;
        }
    }

    if (lockrc==0) unlock(sys,0);
    return found;
}
