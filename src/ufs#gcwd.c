#include "ufs/sys.h"

__asm__("\n&FUNC    SETC 'ufs_get_cwd'");
UFSCWD *ufs_get_cwd(UFS *ufs)
{
    UFSCWD  *cwd    = ufs ? ufs->cwd : NULL;

    return cwd;
}
