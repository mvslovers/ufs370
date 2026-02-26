#include <clibwsa.h>
#include "ufs/sys.h"

static UFSSYS ufssykey = {0};

__asm__("\n&FUNC    SETC 'ufs_sys_get'");
UFSSYS *ufs_sys_get(void)
{
    return (UFSSYS *) __wsaget(&ufssykey, sizeof(UFSSYS));
}
