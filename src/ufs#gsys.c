#include "ufs/sys.h"

__asm__("\n&FUNC    SETC 'ufs_get_sys'");
UFSSYS *ufs_get_sys(UFS *ufs)
{
    UFSSYS  *sys    = ufs ? ufs->sys : NULL;

    return sys;
}
