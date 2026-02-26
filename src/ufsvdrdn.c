#include "ufs/vdisk.h"

__asm__("\n&FUNC    SETC 'ufs_vdisk_read_n'");
INT32 ufs_vdisk_fsread_n(UFSVDISK *vdisk, void *buf, UINT32 block, int count)
{
    return vdisk->io->read_n_fn(vdisk->io->ctx, buf, block, count);
}
