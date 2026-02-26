#include "ufs.h"
#include "ufs/file.h"

__asm__("\n&FUNC    SETC 'ufs_clearerr'");
void ufs_clearerr(UFSFILE *fp)
{
    if (fp) {
        fp->ind     &= ~ FILE_IND_ERROR;
        fp->error   = 0;
    }
}
