#include "ufs/inode.h"

__asm__("\n&FUNC    SETC 'ufs_inode_rel'");
void ufs_inode_rel(UFSMIN *minode)
{
    UFSVDISK *vdisk     = minode->vdisk;
    INT32   lockrc      = -1;
    INT32   lockvdisk   = -1;
    UFSMIN  *m;
    UINT32  count;
    UINT32  n;

    if (!minode) {
        ufs_panic("%s called with NULL handle", __func__);
        return;
    }

    lockvdisk = lock(vdisk, 0);         /* get EXCLUSIVE access     */
    lockrc    = lock(minode,0);         /* get EXCLUSIVE access     */

    /* decrement use count */
    if (minode->usecount > 0) {         /* this *should* always be the case */
        minode->usecount--;             /* decrement use count */
    }

    /* if this is a directory inode AND it has links then keep it in the cache */
    if (ufs_inode_isdir(&minode->dinode) && ufs_inode_nlink(&minode->dinode)) goto quit;

    /* if the use count is now zero then remove this inode from the cache */
    if (minode->usecount==0) {
        count = array_count(&vdisk->minodes);
        for(n=1; n <= count; n++) {
            m = array_get(&vdisk->minodes, n);
            if (!m) continue;

            if (m->inode_number == minode->inode_number) {
                array_del(&vdisk->minodes, n);
                break;
            }
        }

        /* release the inode memory */
        free(minode);
    }

quit:
    if (lockrc==0)    unlock(minode,0);
    if (lockvdisk==0) unlock(vdisk,0);
    return;
}
