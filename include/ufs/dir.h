#ifndef UFS_DIR_H
#define UFS_DIR_H
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

#include "ufs/types.h"      /* atomic types                             */
#include "ufs/time.h"       /* time functions                           */
#include "ufs/disk.h"       /* low level disk, boot block, super block  */
#include "ufs/inode.h"      /* index nodes                              */
#include "ufs/data.h"       /* data block                               */

/* Dirent */
#define	UFS_NAME_MAX        59          /* name max, 59 non 0 character bytes       */
#define	UFS_PATH_MAX        256         /* we have to limit it to something         */
#define	UFS_LINK_MAX        256         /* we have to limit it to something         */
struct ufs_dirent {     /* UFSDIR */
	UINT32      inode_number;           /* 00 inode number                          */
	char        name[UFS_NAME_MAX+1];   /* 04 name + null byte                      */
};                                      /* 40 (64 bytes)                            */

struct ufs_lookup_arg {
    const char  *name;
    char        *buf;
    UINT32      inode_number;
    UFSDIR      *replace;
};

struct ufs_dirlist {    /* UFSDLIST */
    UINT32      inode_number;           /* 00 this directory points to this inode   */
	UINT32      filesize;               /* 04 size of file/directory in bytes       */
	/* UFSTIMEV is now a union. ctime.v1.useconds < 1000000 indicates a UFSTIMEV1
	** time value. Otherwise use UFSTIMEV2 ctime.v2 which is a time64_t struct.
	*/
    mtime64_t   ctime;                  /* 08 creation date     .001 sec resolution */
    mtime64_t   mtime;                  /* 10 modification date .001 sec resolution */
    mtime64_t   atime;                  /* 18 access date       .001 sec resolution */
	UINT16      nlink;                  /* 20 link count                            */
    UINT16      codepage;               /* 22 code page or 0                        */
    char        name[UFS_NAME_MAX+1];   /* 24 name of file/directory                */
    char        owner[8+1];             /* 60 owner name                            */
    char        group[8+1];             /* 69 group name                            */
    char        attr[10+1];             /* 72 "drwxrwxrwx"                          */
    char        unused[3];              /* 7D unused                                */
};                                      /* 80 (128 bytes)                           */

struct ufs_dirdesc {    /* UFSDDSEC */
    UFSMIN      *minode;                /* 00 open directory inode                  */
    UINT32      rec;                    /* 04 record number                         */
    UFSDLIST    **dlist;                /* 08 array of directory records            */
    char        path[UFS_PATH_MAX+1];   /* 0C directory path name                   */
    char        unused[3];              /* 10D align to full word                   */
    void		*arg1;					/* 110 additional args for subroutines		*/
    void		*arg2;					/* 114 additional args for subroutines		*/
};                                      /* 118 (280 bytes)                          */

INT32 ufs_dirent_endian_convert(UFSVDISK *vdisk, UFSDIR *dir, int n)            asm("UFSDIRCV");

void ufs_dirent_filename(char *dst, const char *src)                            asm("UFSDIRFN");

INT32 ufs_directory_add_entry(UFSVDISK *vdisk, UFSMIN *parent_dir, UINT32 ino, const char *srcname)  asm("UFSDIRAD");

INT32 ufs_directory_remove_entry(UFSVDISK *vdisk, UFSMIN *parent_dir, const char *name) asm("UFSDIRRM");

INT32 ufs_directory_replace_entry(UFSVDISK *vdisk,  UFSMIN *self_dir, const char *name, UINT32 ino) asm("UFSDIRRP");



#endif /* UFS_DIR_H */

