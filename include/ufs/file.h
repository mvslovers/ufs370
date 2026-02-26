#ifndef UFS_FILE_H
#define UFS_FILE_H
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

#include <racf.h>               /* security handle (RACF ACEE)              */
#include "ufs/types.h"          /* atomic types                             */
#include "ufs/time.h"           /* time functions                           */
#include "ufs/disk.h"           /* low level disk, boot block, super block  */
#include "ufs/inode.h"          /* index nodes                              */
#include "ufs/data.h"           /* data blocks                              */
#include "ufs/dir.h"            /* directory                                */
#include "ufs/sys.h"            /* system anchor                            */

struct ufs_fileattr {
    char        *owner;             /* 00 owner name                        */
    char        *group;             /* 04 group name                        */
    UINT16      mode;               /* 08 type + permission bits            */
    UINT16      codepage;           /* 0A character code page               */
    UINT32      device;             /* 0C device number (always 0)          */
    UFSTIMEV    atime;              /* 10 access time   (secs,usecs)        */
    UFSTIMEV    mtime;              /* 18 modified time (secs,usecs)        */
    UFSTIMEV    ctime;              /* 20 creation time (secs,usecs)        */
};                                  /* 28 (40 bytes)                        */

INT32 ufs_file_lookup_by_name(UFSMIN *parent_dir, const char *name, UINT32 *ino)                                asm("UFSFILBN");

INT32 ufs_file_allocate(UFSMIN *, const char *, UFSATTR *, UINT32 *)                                            asm("UFSFIALC");

INT32 ufs_file_deallocate(UFSMIN *parent_dir, const char *name)                                                 asm("UFSFIDAL");

INT32 ufs_file_rename(UFSMIN *parent_from, const char *from, UFSMIN *parent_to, const char *to)                 asm("UFSFIREN");

INT32 ufs_file_link(UFSMIN *parent_dir, UFSMIN *p, const char *name)                                            asm("UFSFILNK");

BOOL ufs_file_lookup_by_number(UFSMIN *parent_dir, UINT32 ino, char *buf)                                       asm("UFSFILBZ");

INT32 ufs_file_symlink(UFSMIN *p, const char *target)                                                           asm("UFSFISYM");

struct ufs_file {
    char        eye[8];             /* 00 eye catcher                       */
#define UFS_FILE_EYE    "*UFSFILE"
    UINT32      filepos;            /* 08 position in file                  */
    BYTE        mode;               /* 0C open mode                         */
                /* r or rb              Open file for reading.
                /* w or wb              Truncate to zero length or create file for writing.
                /* a or ab              Append; open or create file for writing at end-of-file.
                /* r+ or rb+ or r+b     Open file for update (reading and writing).
                /* w+ or wb+ or w+b     Truncate to zero length or create file for update.
                /* a+ or ab+ or a+b     Append; open or create file for update, writing at end-of-file.
                */
#define FILE_MODE_READ      0x80    /* ... file opened for input        'r' */
#define FILE_MODE_WRITE     0x40    /* ... file opened for output       'w' */
#define FILE_MODE_BINARY    0x20    /* ... file opened for binary       'b' */
#define FILE_MODE_APPEND    0x08    /* ... writes start at end of file  'a' */
#define FILE_MODE_TRUNC     0x04    /* ... file will be truncated at open   */
#define FILE_MODE_CREATE    0x02    /* ... create file if it doesn't exist  */

    BYTE        ind;                /* 0D indicator/condition               */
#define FILE_IND_EOF        0x80    /* ... end of file was reached on read  */
#define FILE_IND_ERROR      0x40    /* ... error occurred on read/write     */
#define FILE_IND_DIRTY      0x20    /* ... block buffer contents changed    */

    INT16       error;              /* 0E last error                        */

    UFSSYS      *sys;               /* 10 sys handle                        */
    ACEE        *acee;              /* 14 security handle of file opener    */
    UFSMIN      *minode;            /* 18 inode for this file               */
    UFSVDISK    *vdisk;             /* 1C vdisk (from minode->vdisk)        */

    UINT32      blksize;            /* 20 block size (from vdisk->disk)     */
    UINT32      blkno;              /* 24 current block number              */
    UINT32      blkpos;             /* 28 current block buffer position     */
    BYTE        *blkbuf;            /* 2C current block buffer              */

    char        name[UFS_NAME_MAX+1]; /* 30 filename string                 */
    UINT32      unused;             /* 6C unused                            */
};                                  /* 70 (112 bytes)                       */

/* ufs_file_new() allocate a new file handle */
UFSFILE *ufs_file_new(UFSSYS *sys, ACEE *acee, UFSMIN *minode)              asm("UFSFINEW");

/* ufs_file_free() deallocate a file handle */
void ufs_file_free(UFSFILE **file)                                          asm("UFSFIFRE");

/* ufs_file_mode() parse mode string into mode bit values */
BYTE ufs_file_mode(const char *mode)                                        asm("UFSFIMOD");

/* ufs_file_getc() return next character or -1 for EOF, -2 for Error */
INT32 ufs_file_getc(UFSFILE *file)                                          asm("UFSFGETC");

/* ufs_file_getn() get n count characters, returns number of characters */
UINT32 ufs_file_getn(UFSFILE *file, char *buf, UINT32 n)                    asm("UFSFGETN");

/* ufs_file_putc() write character to file, returns EOF on error */
INT32 ufs_file_putc(INT32 c, UFSFILE *file)                                 asm("UFSFPUTC");

/* ufs_file_putn() write characters to file, returns number of character written */
UINT32 ufs_file_putn(UFSFILE *file, const char *buf, UINT32 n)              asm("UFSFPUTN");

/* ufs_file_sync() sync file handle buffer to disk */
void ufs_file_sync(UFSFILE *file)                                           asm("UFSFSYNC");

#endif /* UFS_FILE_H */
