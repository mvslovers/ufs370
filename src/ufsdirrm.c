#include "ufs/dir.h"
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

static INT32 remove_subr(UFSVDISK *vdisk, void *ctx, UINT32 blk, UINT32 sz);

__asm__("\n&FUNC    SETC 'ufs_directory_remove_entry'");
INT32 ufs_directory_remove_entry(UFSVDISK *vdisk, UFSMIN *parent_dir, const char *name)
{
    UFSSB       *sb         = &vdisk->disk->sb;
    void        *buf        = NULL;
    UFSLKARG    lookup_arg  = {0};
    UFSMIN      *minode     = NULL;
    INT32       error       = 0;
    UFSDIR      lastdirent;
    UINT32      lastblk;
    UINT32      sz, lastsz;
    UINT32      pos;

    /* Setup replaced entry. */
    sz = parent_dir->dinode.filesize;
    lastblk = ufs_datablock_last(parent_dir, ufs_inode_filesize(&parent_dir->dinode));
    lastsz = UFS_RESIDUE_BSIZE(sz,sb->blksize_shift);
    pos = lastsz - sizeof(lastdirent);

    buf = ufs_disk_get_buf(vdisk->disk);
    if (!buf) {
        ufs_panic("%s out of memory", __func__);
        error = ENOMEM;
        goto quit;
    }

    if (ufs_vdisk_read(vdisk, buf, lastblk)) {
        ufs_panic("%s read error block %u", __func__, lastblk);
        error = EIO;
        goto quit;
    }

    lastdirent = *((UFSDIR *)((char *)buf + pos));

    /* ufs_panic("%s last dirent=%d %s pos=%d", lastdirent.inode_number, lastdirent.name, pos); */

    lookup_arg.name     = name;
    lookup_arg.replace  = &lastdirent;
    /* Search entry that removed. replace it to last dirent. */
    if ((error = ufs_datablock_foreach(vdisk, parent_dir, remove_subr, &lookup_arg)) != UFS_ITERATOR_BREAK) {
        error = ENOENT;
        goto quit;
    }

    /* Contract dirent entries. */
    ufs_datablock_contract(vdisk, parent_dir, sizeof(lastdirent));
    /* ufs_panic("%s done. (dirent size=%dbyte)", __func__, parent_dir->dinode.filesize); */

    /* Target inode */
    minode = ufs_inode_get(vdisk, lookup_arg.inode_number);
    if (!minode) {
        error = ENOENT;
        goto quit;
    }

    if (ufs_inode_isdir(&minode->dinode)) {
        parent_dir->dinode.nlink--;
        ufs_inode_writeback(parent_dir);
    }

    error = 0;  /* reset error */

quit:
    if (minode) ufs_inode_rel(minode);
    if (buf) ufs_disk_free_buf(vdisk->disk,buf);

    return error;
}

__asm__("\n&FUNC    SETC 'remove_subr'");
static INT32
remove_subr(UFSVDISK *vdisk, void *ctx, UINT32 blk, UINT32 sz)
{
    INT32   ret = 0;
    UFSLKARG *p = (UFSLKARG *)ctx;
    UFSDIR  *dir;
    void    *buf;
    UINT32  i;

    /* ufs_panic("%s match start blk=%x", __func__, blk); */
    buf = ufs_disk_get_buf(vdisk->disk);
    if (!buf) {
        ufs_panic("%s out of memory", __func__);
        ret = ENOMEM;
        goto quit;
    }

    if (ufs_vdisk_read(vdisk, buf, blk)) {
        ufs_panic("%s read error block %u", __func__, blk);
        ret = EIO;
        goto quit;
    }

    dir = (UFSDIR *)buf;

    for (i = 0; i < sz / sizeof(*dir); i++, dir++) {
        /* ufs_panic("%s "%d", __func__, dir->inode_number); */
        if (strncmp(p->name, (const char *)dir->name, UFS_NAME_MAX) == 0) {
            p->inode_number = dir->inode_number;
            /* Replace to last dirent. */
            *dir = *(p->replace); /* disk endian */
            /* Write back. */
            if (ufs_vdisk_write(vdisk, buf, blk))
                ret = EIO;
            else
                ret = UFS_ITERATOR_BREAK;
            break;
        }
    }
quit:
	if (buf) ufs_disk_free_buf(vdisk->disk, buf);

    return ret;
}
