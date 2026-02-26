#ifndef UFS_DISK_H
#define UFS_DISK_H
/*-
 * Copyright (c) 2011 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by UCHIYAMA Yasushi.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <osdcb.h>                      /* DCB struct                           */
#include <osio.h>
#include <osjfcb.h>
#include <osdeb.h>
#include <time.h>
#include <stdlib.h>
#include <errno.h>
#include <clibary.h>
#include <cliblock.h>
#include "ufs/types.h"                  /* our atomic types                     */
#include "ufs/panic.h"
#include <time64.h>

/*
 *  Unix File System
 *  modeled after 7th Edition of Unix(PDP-11) File system definition.
 *
 *     +------------------
 *     |Boot block (4096)	sector [0]
 *     |
 *     +------------------
 *     |Super block (4096)	sector [1]
 *     |
 *     +------------------
 *     |fs_inode (64)       sector [2] start of index nodes
 *         .
 *         .
 *     |
 *     +------------------
 *     |data block (4096)   sector [datablock_start_sector]
 *     |
 *         .
 *         .
 *     |
 *     +------------------	volume size.
 *
 */

#define	UFS_BSIZE       4096                        /* this is the expected disk block size         */
#define	UFS_BSHIFT      12                          /* number of bits for 4096 expressed in binary  */

#define	UFS_ROUND_BSIZE(x,blksize)                  \
    ((((x) + ((blksize) - 1)) & ~((blksize) - 1)))

#define	UFS_TRUNC_BSIZE(x,blksize)  ((x) & ~((blksize) - 1))

#define	UFS_RESIDUE_BSIZE(x,bshift)                 \
    ((x) - ((((x) - 1) >> (bshift)) << (bshift)))

/* special disk sectors (physical blocks) */
#define	UFS_BOOTBLOCK_SECTOR    0                   /* boot specific information                    */
#define	UFS_SUPERBLOCK_SECTOR   1                   /* file system information                      */
#define	UFS_ILIST_SECTOR        2                   /* index nodes start here                       */

/* boot block (physical block 0) for disk dataset */
struct ufs_boot {                                   /* boot block physical block 0                  */
    UINT16          type;                           /* 00 file system type                          */
#define UFS_DISK_TYPE_RAW           0               /* ... raw/unformatted disk                     */
#define UFS_DISK_TYPE_UNKNOWN       1               /* ... an unknown disk type                     */
#define UFS_DISK_TYPE_UFS           2               /* ... unix (like) file system                  */
    UINT16          check;                          /* 02 ~type (type+check==0xFFFF)                */
    UINT16          blksize;                        /* 04 physical block size                       */
    UINT16          padding;                        /* 06 align to full word                        */
};                                                  /* 08                                           */

/* boot block extension (follows ufs_boot area in physical block 0) for disk dataset */
struct ufs_boot_ext {
    time64_t        create_time;                    /* 08 disk created time                         */
    time64_t        update_time;                    /* 10 disk updated time                         */
    BYTE            version;                        /* 18 disk format version                       */
#define UFS_DISK_BOOT_0             0               /* ... disk format 0 (no extension)             */
#define UFS_DISK_BOOT_1             1               /* ... disk format 1 (has extension)            */
    BYTE            unused1[3];                     /* 19 unused/available                          */
    UINT32          unused2;                        /* 1C unused/available                          */
    UINT32          unused3[56];                    /* 20 unused/available                          */
};                                                  /* 100 (256 bytes)                              */

/* inode and free block cache. values chosen to make super block fit in 512 byte area */
#define	UFS_MAX_FREEBLOCK       51
#define	UFS_MAX_FREEINODE       64

/* super block (physical block 1) for disk dataset */
struct ufs_superblock {
    /* Note:
    ** UFS_ILIST_SECTOR through (datablock_start_sector - 1) are ilist (inodes)
    */
	UINT32          datablock_start_sector;         /* 00 sector number of first data block on disk */
	UINT32          volume_size;                    /* 04 disk volume size in disk sectors          */
	BYTE            lock_freeblock;                 /* 05 free block lock                           */
	BYTE            lock_freeinode;                 /* 06 free inode lock                           */
	BYTE            modified;                       /* 07 modified flag                             */
	BYTE            readonly;                       /* 08 read only flag                            */
	time_t          update_time;                    /* 0C update time stamp                         */
	UINT32          total_freeblock;                /* 10 total free blocks                         */
	UINT32          total_freeinode;                /* 14 total free inodes                         */
    time_t          create_time;                    /* 18 disk create time (format time)            */
	UINT32          nfreeblock;	                    /* 1C # of freeblock in superblock cache.       */
	UINT32          freeblock[UFS_MAX_FREEBLOCK];   /* 20 free block cache.                         */
	UINT32          nfreeinode;	                    /* EC # of free inode in superblock cache.      */
	UINT32          freeinode[UFS_MAX_FREEINODE];   /* F0 free inode cache.                         */
	UINT32          inodes_per_block;               /* 1F0 inodes per block                         */
	UINT32          blksize_shift;                  /* 1F4 block size shift bits(UFS_BSHIFT)        */
	UINT32          ilist_sector;                   /* 1F8 inode list starts here                   */
	UINT32          unused3;                        /* 1FC unused                                   */
};                                                  /* 200 (512 bytes)                              */

/* unix style file system, emulated physical disk (BDAM dataset in MVS) */
struct ufs_disk {                                   /* disk dataset handle                          */
    char            eye[8];                         /* 00 eye catcher for dumps                     */
#define UFSDISK_EYE "UFSDISK"                       /* ... eye catcher for dumps                    */
    DCB             *dcb;                           /* 08 DCB for opened disk dataset               */
    UINT16          blksize;                        /* 0C physical block size (from DCB)            */
    BYTE            readonly;                       /* 0E DISP=SHR, don't update this disk          */
    BYTE            unused;                         /* 0F unused                                    */
    char            **buf;                          /* 10 array of disk buffers                     */
    char            ddname[12];                     /* 14 DD name of disk dataset                   */
    char            dsname[48];                     /* 20 dataset name of disk dataset              */
    UFSBOOT         boot;                           /* 50 boot block for disk dataset (8 bytes)     */
    UFSSB           sb;                             /* 58 super block                 (512 bytes)   */
    UFSBOOTE        *boote;                         /* 258 boot extension for disk                  */
};                                                  /* 25C (604 bytes)                              */

/* open a formatted disk dataset for I/O */
UFSDISK *ufs_disk_open(const char *ddname, int check_sb)                        asm("UFSDKOPN");

/* read a physical block from disk dataset */
INT32 ufs_disk_read(UFSDISK *disk, void *buf, UINT32 block)                     asm("UFSDKRD");

/* read a physical block from disk dataset */
INT32 ufs_disk_read_n(UFSDISK *disk, void *buf, UINT32 block, int count)        asm("UFSDKRDN");

/* read super block from disk dataset */
INT32 ufs_disk_read_super(UFSDISK *disk, UFSSB *sb)                             asm("UFSDKRSB");

/* write a physical block to disk dataset */
INT32 ufs_disk_write(UFSDISK *disk, void *buf, UINT32 block)                    asm("UFSDKWT");

/* write a physical block to disk dataset */
INT32 ufs_disk_write_n(UFSDISK *disk, void *buf, UINT32 block, int count)       asm("UFSDKWTN");

/* write super block to disk dataset */
INT32 ufs_disk_write_super(UFSDISK *disk, UFSSB *sb)                            asm("UFSDKWSB");

#if 1
/* write boot block to disk dataset */
INT32 ufs_disk_write_boot(UFSDISK *disk)                                        asm("UFSDKWBT");
#endif

/* close disk dataset - deallocates UFSDISK handle */
void ufs_disk_close(UFSDISK **disk)                                             asm("UFSDKCLS");

/* get buffer for disk I/O */
void *ufs_disk_get_buf(UFSDISK *disk)                                           asm("UFSDKGBF");

/* free disk I/O buffer */
void ufs_disk_free_buf(UFSDISK *disk, void *buf)                                asm("UFSDKFBF");

/* check super block for sane values, return true if not sane */
INT32 ufs_disk_check_super(UFSDISK *disk, UFSSB *sb, int quiet)                 asm("UFSDKCSB");

/* UFSDKOPN.C: */
#define UFSDK001E "UFSDK001E Unable to allocate disk handle"
#define UFSDK002E "UFSDK002E Unable to allocate disk DCB handle"
#define UFSDK003E "UFSDK003E Unable to open BDAM disk dataset for %s"
#define UFSDK004E "UFSDK004E Block size must be a multiple of 512 for disk dataset %s"
#define UFSDK005E "UFSDK005E Unable to allocate %u byte buffer"
#define UFSDK006E "UFSDK006E Unable to read boot block for %s"
#define UFSDK007E "UFSDK007E Block size mismatch in boot block for %s"
#define UFSDK008E "UFSDK008E type check mismatch in boot block for %s"
#define UFSDK009E "UFSDK009E Unable to read super block for %s"
#define UFSDK010E "UFSDK010E Invalid super block for %s"

/* UFSDKCLS.C: */
#define UFSDK011E "UFSDK011E write of super block failed, rc=%d"

#endif /* UFS_DISK_H */
