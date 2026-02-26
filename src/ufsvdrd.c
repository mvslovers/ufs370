#include "ufs/vdisk.h"

__asm__("\n&FUNC    SETC 'ufs_vdisk_read'");
INT32 ufs_vdisk_read(UFSVDISK *vdisk, void *buf, UINT32 block)
{
    return vdisk->io->read_fn(vdisk->io->ctx, buf, block);
}
