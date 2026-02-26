#include <clibcrt.h>
#include <racf.h>
#include "ufs/inode.h"

__asm__("\n&FUNC    SETC 'ufs_inode_auth_exec'");
BOOL ufs_inode_auth_exec(ACEE *acee, UFSMIN *minode)
{
    return ufs_inode_auth(acee, minode, UFSACC_EXEC);
}
