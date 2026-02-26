#include "ufs/vdisk.h"

__asm__("\n&FUNC    SETC 'ufs_vdisk_write_n'");
INT32 ufs_vdisk_write_n(UFSVDISK *vdisk, void *buf, UINT32 block, int count)
{
    if (vdisk->disk->readonly) return EROFS;

    return vdisk->io->write_n_fn(vdisk->io->ctx, buf, block, count);
}
