#include "ufs/sys.h"

__asm__("\n&FUNC    SETC 'ufs_set_acee'");
ACEE *ufs_set_acee(UFS *ufs, ACEE *acee)
{
    ACEE    *ret    = NULL;

    if (ufs) {
        /* if the previous ACEE was created by ufs_signon then we need to release it now */
        if (ufs->acee && ufs->flags & UFS_ACEE_SIGNON) racf_logout(&ufs->acee);

        /* return the previous ACEE handle or NULL */
        ret = ufs->acee;
        /* reset ACEE flags */
        ufs->flags &= ~(UFS_ACEE_DEFAULT | UFS_ACEE_SIGNON | UFS_ACEE_USER);

        /* save the new ACEE handle or NULL */
        ufs->acee = acee;

        /* set the ACEE flags */
        if (acee) ufs->flags |= UFS_ACEE_USER;
    }

    return ret;
}
