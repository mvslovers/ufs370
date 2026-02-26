#include "ufs/name.h"
#include "ufs/sys.h"

static int name_sort(const void *v1, const void *v2);

__asm__("\n&FUNC    SETC 'ufs_name_sort'");
INT32 ufs_name_sort(void)
{
    INT32   error       = 0;
    UFSSYS  *sys        = ufs_sys_get();
    UFSNAME **names;
    UINT32  count;
    INT32   lockrc;
    UFSNAME *name;

    if (!sys) return EPERM;

    lockrc = lock(sys,0);
    if (lockrc!=0 && lockrc!=8) {
        ufs_panic("%s lock failure", __func__);
        error = EPERM;
        goto quit;
    }

    names = sys->names;
    count = arraycount(&names);

    if (count > 1) {
        qsort(names, count, sizeof(UFSNAME *), name_sort);
    }

quit:
    if (lockrc==0) unlock(sys,0);
    return error;
}

__asm__("\n&FUNC    SETC 'name_sort'");
static int name_sort(const void *v1, const void *v2)
{
    const UFSNAME *p1 = *(UFSNAME**) v1;
    const UFSNAME *p2 = *(UFSNAME**) v2;
    const char    *s1 = p1 && p1->prefix ? p1->prefix : "";
    const char    *s2 = p2 && p2->prefix ? p2->prefix : "";
    int           l1    = strlen(s1);
    int           l2    = strlen(s2);
    int           diff  = l1 - l2;

    /* we want longest names first, then ascending */
    if (diff==0) {
        /* equal length names */
        diff = strcmp(s2, s1);  /* yep, we want them reversed */
    }

    /* we're doing a reverse sort (longest prefix will be first in our names array) */
    return (diff *-1);  /* change sign */
}
