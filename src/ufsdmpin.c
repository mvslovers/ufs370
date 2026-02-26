#include "ufs/inode.h"

__asm__("\n&FUNC    SETC 'ufs_dump_inodes'");
void ufs_dump_inodes(UFSVDISK *vdisk)
{
    UFSMIN  *minode;
    UINT32  count;
    UINT32  n;

#if 1
    char    caller[64] = "";

    __caller(caller);
    wtof("%s for caller %s", __func__, caller);
#endif
    if (!vdisk) return;
    if (!vdisk->minodes) return;

    count = array_count(&vdisk->minodes);
    if (!count) return;

    for(n=0; n < count; n++) {
        minode = vdisk->minodes[n];
        if (!minode) continue;
        ufs_inode_dump(minode);
    }
}
