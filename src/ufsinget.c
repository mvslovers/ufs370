#include "ufs/inode.h"

__asm__("\n&FUNC    SETC 'ufs_inode_get'");
UFSMIN *ufs_inode_get(UFSVDISK *vdisk, UINT32 ino)
{
    INT32   error   = 0;
    UFSMIN  *minode = 0;
    UINT32  count;
    UINT32  n;
    INT32   lockvdisk;

    lockvdisk = lock(vdisk, 0);         /* get EXCLUSIVE access     */

    count = array_count(&vdisk->minodes);
    for(n=0; n < count; n++) {
        minode = vdisk->minodes[n];
        if (!minode) continue;
        if (minode->inode_number == ino) goto found;
    }

    /* inode not in cache, allocate new inode handle and add to cache */
    minode = calloc(1, sizeof(UFSMIN));
    if (!minode) {
        ufs_panic("%s out of memory", __func__);
        goto quit;
    }

    /* load inode handle from disk */
    if ((error = ufs_inode_load(vdisk, minode, ino))) {
        ufs_panic("%s load failure %d", __func__, error);
        free(minode);
        minode = NULL;
        goto quit;
    }

    /* add inode handle to cache */
    array_add(&vdisk->minodes, minode);

found:
    /* bump the use count */
    ufs_inode_use(minode);

quit:
    if (lockvdisk==0) unlock(vdisk,0);

    return minode;
}
