#include "ufs/sys.h"

__asm__("\n&FUNC    SETC 'ufs_signoff'");
void ufs_signoff(UFS *ufs)
{
    if (ufs) {
        /* if the previous ACEE was created by ufs_signon then we need to release it now */
        if (ufs->acee && ufs->flags & UFS_ACEE_SIGNON) {
            racf_logout(&ufs->acee);

            /* reset ACEE flags */
            ufs->flags &= ~UFS_ACEE_SIGNON;
        }
    }

    return;
}
