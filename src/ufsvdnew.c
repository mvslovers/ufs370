#include "ufs/vdisk.h"

/* create instance of UFSVDISK handle */
__asm__("\n&FUNC    SETC 'ufs_vdisk_new'");
UFSVDISK *ufs_vdisk_new(UFSDISK *disk, UFSIO *io)
{
    UFSVDISK    *vdisk = calloc(1, sizeof(UFSVDISK));

    if (vdisk) {
        strcpy(vdisk->eye, UFSVDISK_EYE);
        vdisk->disk     = disk;             /* disk dataset, blksize, super block   */
        vdisk->io       = io;               /* I/O disk or pager function pointers  */
    }

    return vdisk;
}
