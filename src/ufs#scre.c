#include "ufs/sys.h"
#include "ufs/file.h"

__asm__("\n&FUNC    SETC 'ufs_set_create_perm'");
UINT32 ufs_set_create_perm(UFS *ufs, UINT32 perm)
{
    UINT32  ret = 0;

    if (ufs) {
        ret = ufs->create_perm;
        ufs->create_perm = perm & 0777;
    }

    return ret;
}
