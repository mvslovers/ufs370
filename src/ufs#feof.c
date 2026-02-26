#include "ufs.h"
#include "ufs/file.h"

__asm__("\n&FUNC    SETC 'ufs_feof'");
INT32 ufs_feof(UFSFILE *fp)
{
    INT32   eof = 0;

    if (fp && fp->ind & FILE_IND_EOF) {
        eof = 1;
    }

    return eof;
}
