#include "ufs/sys.h"
#include "ufs/file.h"

__asm__("\n&FUNC    SETC 'ufs_get_create_perm'");
UINT32 ufs_get_create_perm(UFS *ufs)
{
    UINT32  perm = 0;

    if (ufs) {
        perm = ufs->create_perm;
    }

    return perm;
}
