#include "ufs/sys.h"

__asm__("\n&FUNC    SETC 'ufs_set_sys'");
UFSSYS *ufs_set_sys(UFS *ufs, UFSSYS *sys)
{
    UFSSYS  *ret    = NULL;

    if (ufs) {
        ret = ufs->sys;
        ufs->sys = sys;
    }

    return ret;
}
