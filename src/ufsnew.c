#include "ufs/sys.h"

__asm__("\n&FUNC    SETC 'ufsnew'");
UFS  *ufsnew(void)
{
    UFS     *ufs    = calloc(1, sizeof(UFS));

    if (ufs) {
        strcpy(ufs->eye, UFSEYE);
        ufs->sys        = ufs_sys_get();
        ufs->cwd        = ufs_cwd_new();
        ufs->acee       = racf_get_acee();
        if (ufs->acee) ufs->flags |= UFS_ACEE_DEFAULT;
                                    /* Own Grp World */
        ufs->create_perm = 0755;    /* rwx r-x r-x   */
    }

    return ufs;
}
