#include "ufs.h"
#include "ufs/file.h"

__asm__("\n&FUNC    SETC 'ufs_ferror'");
INT32 ufs_ferror(UFSFILE *fp)
{
    INT32   error   = 0;

    if (fp && fp->ind & FILE_IND_ERROR) {
        error = (INT32) fp->error;
    }

    return error;
}
