#include "ufs/vdisk.h"
#include "ufs/inode.h"

__asm__("\n&FUNC    SETC 'ufs_vdisk_free'");
void ufs_vdisk_free(UFSVDISK **vdisk)
{
    UFSVDISK *v;
    UINT32  count;
    UINT32  n;
    UFSMIN  *m;

    if (vdisk && *vdisk) {
        /* free the inode cache */
        v = *vdisk;
        if (v->minodes) {
            count = array_count(&v->minodes);
            for(n=count; n > 0; n--) {
                m = array_del(&v->minodes, n);
                if (!m) continue;
                if (m->flags & MINODE_FLAG_DIRTY) {
                    ufs_inode_writeback(m);
                }
                ufs_inode_rel(m);
            }
            array_free(&v->minodes);
        }
        free(v);
        *vdisk = NULL;
    }
}
