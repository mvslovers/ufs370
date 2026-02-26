#include <clibcrt.h>
#include <racf.h>
#include "ufs/inode.h"

__asm__("\n&FUNC    SETC 'ufs_inode_auth_read'");
BOOL ufs_inode_auth_read(ACEE *acee, UFSMIN *minode)
{
    return ufs_inode_auth(acee, minode, UFSACC_READ);
}
