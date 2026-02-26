#include "ufs/vdisk.h"

__asm__("\n&FUNC    SETC 'ufs_vdisk_write'");
INT32 ufs_vdisk_write(UFSVDISK *vdisk, void *buf, UINT32 block)
{
    if (vdisk->disk->readonly) return EROFS;

    return vdisk->io->write_fn(vdisk->io->ctx, buf, block);
}
