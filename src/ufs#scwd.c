#include "ufs/sys.h"

__asm__("\n&FUNC    SETC 'ufs_set_cwd'");
UFSCWD *ufs_set_cwd(UFS *ufs, UFSCWD *cwd)
{
    UFSCWD  *ret    = NULL;

    if (ufs) {
        ret = ufs->cwd;
        ufs->cwd = cwd;
    }

    return ret;
}
