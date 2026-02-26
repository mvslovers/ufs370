#include "ufs/sys.h"

__asm__("\n&FUNC    SETC 'ufs_signon'");
INT32 ufs_signon(UFS *ufs, const char *userid, const char *password, const char *group)
{
    INT32   rc    = 0;
    ACEE    *acee = racf_login(userid, password, group, &rc);

    if (ufs && acee) {
        /* if the previous ACEE was created by ufs_signon then we need to release it now */
        if (ufs->acee && ufs->flags & UFS_ACEE_SIGNON) racf_logout(&ufs->acee);

        /* reset ACEE flags */
        ufs->flags &= ~(UFS_ACEE_DEFAULT | UFS_ACEE_SIGNON | UFS_ACEE_USER);

        /* save the new ACEE handle or NULL */
        ufs->acee = acee;

        /* set the ACEE flags */
        if (acee) ufs->flags |= UFS_ACEE_SIGNON;
    }

    return rc;
}
