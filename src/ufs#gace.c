#include "ufs/sys.h"

__asm__("\n&FUNC    SETC 'ufs_get_acee'");
ACEE *ufs_get_acee(UFS *ufs)
{
    ACEE    *acee   = ufs ? ufs->acee : NULL;

    return acee;
}
