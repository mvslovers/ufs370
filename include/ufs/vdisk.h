#ifndef UFS_VDISK_H
#define UFS_VDISK_H

#include "ufs/disk.h"
#include "ufs/io.h"

struct ufsvdisk {
    char        eye[8];             /* 00 eye catcher for dumps             */
#define UFSVDISK_EYE "UFSVDISK"     /* ... eye catcher for dumps            */
    UFSDISK     *disk;              /* 08 disk dataset, blksize, super block*/
    UFSIO       *io;                /* 0C I/O function pointers             */
    UFSMIN      **minodes;          /* 10 inode cache                       */
    UFSMIN      *mounted_minode;    /* 14 mounted on this inode             */
};                                  /* 18 (24 bytes)                        */

/* create instance of UFSVDISK handle */
UFSVDISK *ufs_vdisk_new(UFSDISK *disk, UFSIO *io)                               asm("UFSVDNEW");

/* free instance of UFSVDISK handle */
void ufs_vdisk_free(UFSVDISK **vdisk)                                           asm("UFSVDFRE");

/* read block from cache or disk */
INT32 ufs_vdisk_read(UFSVDISK *vdisk, void *buf, UINT32 block)                  asm("UFSVDRD");

/* read one or more blocks from cache or disk */
INT32 ufs_vdisk_fsread_n(UFSVDISK *vdisk, void *buf, UINT32 block, int count)   asm("UFSVDRDN");

/* write block to cache and disk */
INT32 ufs_vdisk_write(UFSVDISK *vdisk, void *buf, UINT32 block)                 asm("UFSVDWT");

/* write one or more blocks to cache and disk */
INT32 ufs_vdisk_write_n(UFSVDISK *vdisk, void *buf, UINT32 block, int count)    asm("UFSVDWTN");

#endif /* UFS_VDISK_H */
