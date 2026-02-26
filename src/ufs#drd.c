#include "ufs/sys.h"
#include "ufs/file.h"

__asm__("\n&FUNC    SETC 'ufs_dirread'");
UFSDLIST *ufs_dirread(UFSDDESC *ddesc)
{
    if (!ddesc) return NULL;

    return array_get(&ddesc->dlist, ++ddesc->rec);
}
