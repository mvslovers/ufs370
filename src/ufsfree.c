#include "ufs/sys.h"

__asm__("\n&FUNC    SETC 'ufsfree'");
void ufsfree(UFS **ppufs)
{
    if (ppufs && *ppufs) {
        UFS *ufs = *ppufs;

        if (ufs->cwd)   ufs_cwd_free(&ufs->cwd);
        if (ufs->acee && (ufs->flags & UFS_ACEE_SIGNON)) {
            racf_logout(&ufs->acee);
            ufs->flags &= ~UFS_ACEE_SIGNON;
        }

        free(ufs);
        *ppufs = NULL;
    }
}
