#ifndef UFS_INODE_H
#define UFS_INODE_H
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

#include <acee.h>           /* RACF ACEE and CONNGRP structures         */
#include <clibos.h>         /* __inc(), __uinc(), __dec(), __udec()     */
#include "ufs/types.h"      /* atomic types                             */
#include "ufs/time.h"       /* time functions                           */
#include "ufs/disk.h"       /* low level disk, boot block, super block  */
#include "ufs/vdisk.h"      /* virtual disk                             */

/* inode data node address list */
#define	UFS_NADDR           19                      /* number of data addresses                     */
#define	UFS_NADDR_DIRECT    16                      /* number of direct addresses for data blocks   */
#define	UFS_NADDR_INDEX1    16                      /* this one is single indirect address block    */
#define	UFS_NADDR_INDEX2    17                      /* this one is double indirect address block    */
#define	UFS_NADDR_INDEX3    18                      /* this one is triple indirect address block    */

/* inode on disk */
struct ufs_dinode {
    UINT16  mode;                                   /* 00 type of file                              */
	UINT16  nlink;	                                /* 02 [DIR] # of child directories. [REG] link count. */
	UINT32  filesize;	                            /* 04 file size in bytes                        */
	UFSTIMEV ctime;                                 /* 08 creation time {secs,usecs}                */
	UFSTIMEV mtime;                                 /* 10 modified time {secs,usecs}                */
	UFSTIMEV atime;                                 /* 18 access time {secs,usecs}                  */
    char    owner[8+1];                             /* 20 owner user id + null byte                 */
    char    group[8+1];                             /* 29 group name + null byte                    */
    UINT16  codepage;                               /* 32 code page                                 */
	UINT32  addr[UFS_NADDR];                        /* 34 data node address list                    */
	/*for device node: addr[0] is major << 8 | minor. */
};                                                  /* 80 (128 bytes)                               */

/* inode in memory */
struct ufs_minode {
    UINT32  inode_number;                           /* 00 inode location                            */
    UFSVDISK *vdisk;                                /* 04 virtual disk handle                       */
    BYTE    type;                                   /* 08 type of inode                             */
#define MINODE_TYPE_FILE        0x80                /* ... this is a normal file inode              */
#define MINODE_TYPE_DIR         0x40                /* ... this is a normal directory inode         */
#define MINODE_TYPE_FSROOT      0x20                /* ... this is a file system root directory     */
#define MINODE_TYPE_MOUNTPOINT	0x10				/* ... this is a mount point for another vdisk	*/

    BYTE    flags;                                  /* 09 processing flags                          */
#define MINODE_FLAG_APPEND      0x80                /* ... append mode output                       */
#define MINODE_FLAG_RDONLY      0x40                /* ... file or dir inode is read only           */
#define MINODE_FLAG_DIRTY       0x20                /* ... inode has changed and not saved to disk  */

    UINT16  usecount;                               /* 0A reference counter                         */
    UFSVDISK *mounted_vdisk;                        /* 0C mounted vdisk on this inode               */
    UFSDIN  dinode;                                 /* 10 disk inode                                */
};                                                  /* 90 (144 bytes)                               */

#define	ufs_inode_allocated(i)  ((i)->mode)
#define	ufs_inode_nlink(i)      ((i)->nlink)
#define	ufs_inode_filesize(i)   ((i)->filesize)
/* V7 original */
#define	ufs_inode_isdir(i)      (((i)->mode & UFS_IFMT) == UFS_IFDIR)
#define	ufs_inode_isfile(i)     (((i)->mode & UFS_IFMT) == UFS_IFREG)
#define	ufs_inode_iscdev(i)     (((i)->mode & UFS_IFMT) == UFS_IFCHR)
#define	ufs_inode_isbdev(i)     (((i)->mode & UFS_IFMT) == UFS_IFBLK)
/* 2BSD extension (implementation is different) */
#define	ufs_inode_islnk(i)      (((i)->mode & UFS_IFMT) == UFSBSD_IFLNK)
#define	ufs_inode_issock(i)     (((i)->mode & UFS_IFMT) == UFSBSD_IFSOCK)
/* NetBSD Extension */
#define	ufs_inode_isfifo(i)     (((i)->mode & UFS_IFMT) == UFSBSD_IFFIFO)


/* daddr index. */
#define	UFS_DADDR_PER_BLOCK(blksize)   (((blksize) / sizeof(UINT32)))
struct ufs_freeblock {
    UINT32  nfreeblock;                             /* 00 free block count                          */
    UINT32  freeblock[UFS_MAX_FREEBLOCK];           /* 04 free blocks (51 blocks)                   */
};                                                  /* D0 (208 bytes)                               */

/* Inode */
#define	UFS_BALBLK_INODE    1                       /* monument (never used)                        */
#define	UFS_ROOT_INODE      2                       /* inode of root directory                      */

/* calculate how many inodes fit in a sector (physical block) */
#define	UFS_INODE_PER_BLOCK(blksize) \
    ((blksize) / sizeof(UFSDIN))
static __inline UINT32 ufs_inode_per_block(UINT32 blksize)
{
    return (UINT32) (blksize / sizeof(UFSDIN));
}

/* calculate the total number of inodes */
#define	UFS_MAX_INODE(blksize,datablock_start_sector) \
    (((datablock_start_sector) - UFS_ILIST_SECTOR) * UFS_INODE_PER_BLOCK(blksize))
static __inline UINT32 ufs_max_inode(UINT32 blksize, UINT32 datablock_start_sector)
{
    return (UINT32) (datablock_start_sector - UFS_ILIST_SECTOR) * ufs_inode_per_block(blksize);
}

/* calculate the total number of inode sectors (physical blocks) */
#define	UFS_ILISTBLK_MAX(blksize,datablock_start_sector) \
    (UFS_MAX_INODE((blksize),(datablock_start_sector)) / UFS_INODE_PER_BLOCK(blksize))
static __inline UINT32 ufs_ilistblk_max(UINT32 blksize, UINT32 datablock_start_sector)
{
    return (UINT32) (ufs_max_inode(blksize, datablock_start_sector) / ufs_inode_per_block(blksize));
}

#define UFS_UMASK       0755        /* 0x01ED gets or'd with mode       */

/* File type */
#define	UFS_IFMT        0170000	    /* 0xF000 File type mask            */
#define	UFS_IFCHR       0020000	    /* 0x2000 character device          */
#define	UFS_IFDIR       0040000     /* 0x4000 directory                 */
#define	UFS_IFBLK       0060000     /* 0x6000 block device              */
#define	UFS_IFREG       0100000     /* 0x8000 file.                     */

/* Obsoleted file type. */
#define	UFS_IFMPC       0030000     /* 0x3000 multiplexed char special  */
#define	UFS_IFMPB       0070000	    /* 0x7000 multiplexed block special */

/* Doesn't appear original V7 filesystem. Found at 2.10BSD. */
#define	UFSBSD_IFLNK    0120000	    /* 0xA000 symbolic link             */
#define	UFSBSD_IFSOCK   0140000     /* 0xC000 socket                    */

/* Doesn't appear original V7 filesystem. NetBSD. */
#define	UFSBSD_IFFIFO   0010000     /* 0x1000 Named pipe.               */

#define	UFSBSD_MAXSYMLINKLEN    UFS_BSIZE

/* Free inode access ops. */
INT32 ufs_inode_allocate(UFSVDISK *vdisk, UINT32 *inode)                            asm("UFSINALC");
void ufs_inode_deallocate(UFSVDISK *vdisk, UINT32 inode)                            asm("UFSINDAL");

/* Load inode into memory */
INT32 ufs_inode_load(UFSVDISK *vdisk, UFSMIN *minode, UINT32 inode)                 asm("UFSINLOD");

/* Write back inode to disk. */
INT32 ufs_inode_writeback(UFSMIN *minode)                                           asm("UFSINWBK");

/* Copy disk inode image to memory inode image */
void ufs_inode_setup_memory_image(UFSVDISK *vdisk, UFSMIN *minode, UFSDIN *dinode)  asm("UFSINSMI");

/* Copy memory inode image to disk inode image */
void ufs_inode_setup_disk_image(UFSVDISK *vdisk, UFSMIN *minode, UFSDIN *dinode)    asm("UFSINSDI");

/* inode santity check */
INT32 ufs_inode_number_sanity(UFSVDISK *vdisk, UINT32 inode)                        asm("UFSINSAN");

/* Util. */
void ufs_inode_chmod(UFSMIN *minode, UINT32 mode)                                   asm("UFSINCHM");
void ufs_inode_dump(UFSMIN *minode)                                                 asm("UFSINDMP");

/* Loop over all inode in ilist. */
INT32 ufs_ilist_foreach(UFSVDISK *vdisk,
                        INT32 (*)(UFSVDISK *, void *, UFSMIN *, UINT32), void *)    asm("UFSINFOR");

/* Fill free inode to superblock cache. */
INT32 ufs_freeinode_update(UFSVDISK *vdisk)                                         asm("UFSINFUP");

/* given an inode number, calculate the sector (block) and offset (array index) for that inode */
INT32 ufs_inode_inquire_disk_location(UFSVDISK *vdisk, UINT32 ino, UINT32 *block, UINT32 *offset) asm("UFSINIDL");

/* verify user represented by ACEE is allowed to access this inode. ACEE can be NULL */
typedef enum ufsacc {
    UFSACC_READ=4,    /* read access */
    UFSACC_WRITE=2,   /* write access */
    UFSACC_EXEC=1     /* execute/directory list access */
} UFSACC;
BOOL ufs_inode_auth(ACEE *acee, UFSMIN *minode, UINT32 ufsacc)                      asm("UFSINACC");

BOOL ufs_inode_auth_read(ACEE *acee, UFSMIN *minode)                                asm("UFSINARD");

BOOL ufs_inode_auth_write(ACEE *acee, UFSMIN *minode)                               asm("UFSINAWT");

BOOL ufs_inode_auth_exec(ACEE *acee, UFSMIN *minode)                                asm("UFSINAEX");

/* Cached inodes functions */

/* Get inode from cache or disk */
UFSMIN *ufs_inode_get(UFSVDISK *vdisk, UINT32 ino)                                  asm("UFSINGET");

/* Get inode from cache or disk */
UFSMIN *ufs_inode_use(UFSMIN *minode)                                               asm("UFSINUSE");

/* Release inode obtained by ufs_inode_get() */
void ufs_inode_rel(UFSMIN *minode)                                                  asm("UFSINREL");

void ufs_dump_inodes(UFSVDISK *vdisk)                                               asm("UFSDMPIN");

#define UFSIN001E "UFSIN001E invalid inode#%u (%u-%u)"

#endif /* UFS_INODE_H */
