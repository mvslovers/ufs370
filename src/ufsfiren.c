#include "ufs/file.h"
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

static INT32 can_dirmove(UFSVDISK *vdisk, UINT32 from_ino, UINT32 to_ino);

__asm__("\n&FUNC    SETC 'ufs_file_rename'");
INT32 ufs_file_rename(UFSMIN *parent_from, const char *from, UFSMIN *parent_to, const char *to)
{
    UFSVDISK *vdisk = parent_from->vdisk;
    INT32   error   = 0;
    UINT32  from_ino, to_ino;
    UFSMIN  *minode = NULL;
    INT32   dir_move;

    /* Check source file */
    if ((error = ufs_file_lookup_by_name(parent_from, from, &from_ino))) {
        /* ufs_panic("%s %s don't exists", __func__, from); */
        return error;
    }

    minode = ufs_inode_get(vdisk, from_ino);
    if (!minode) {
        error = ENOENT;
        goto quit;
    }
    dir_move = ufs_inode_isdir(&minode->dinode);

    /* Check target file */
    error = ufs_file_lookup_by_name(parent_to, to, &to_ino);
    if (error == 0) {	/* found */
        /* ufs_panic("%s %s already exists", __func__, to); */
        if ((error = ufs_file_deallocate(parent_to, to))) {
            /* ufs_panic("%s %s can't remove %d", __func__, to, error); */
            goto quit;
        }
    }
    else if (error != ENOENT) {
        /* ufs_panic("%s error=%d", __func__, error); */
        goto quit;
    }

    /* Check directory hierarchy. t_vnops rename_dir(5) */
    if (dir_move && (error = can_dirmove(vdisk, from_ino, parent_to->inode_number))) {
        /* ufs_panic("%s dst '%s' is child dir of '%s'. error=%d", __func__, to, from, error); */
        goto quit;
    }

    if ((error = ufs_directory_add_entry(vdisk, parent_to, from_ino, to))) {
        /* ufs_panic("%s can't add entry", __func__); */
        goto quit;
    }

    if ((error = ufs_directory_remove_entry(vdisk, parent_from, from))) {
        /* ufs_panic("%s can't remove entry", __func__); */
        goto quit;
    }

    if (dir_move && (parent_from != parent_to)) {
        /* If directory move, update ".." */
        if ((error = ufs_directory_replace_entry(vdisk, minode, "..", parent_to->inode_number))) {
            /* ufs_panic("%s can't replace parent dir", __func__); */
            goto quit;
        }
        ufs_inode_writeback(minode);
    }

quit:
    if (minode) ufs_inode_rel(minode);
    return error;
}

struct lookup_parent_arg {
    UINT32 parent_ino;
};

static INT32 lookup_parent_from_dir_subr(UFSVDISK *vdisk, void *ctx, UINT32 blk, UINT32 sz);

__asm__("\n&FUNC    SETC 'can_dirmove'");
static INT32
can_dirmove(UFSVDISK *vdisk, UINT32 from_ino, UINT32 to_ino)
{
    UFSMIN  minode;
    UINT32  parent;
    INT32   error;

    /* Start dir. */
    if ((error = ufs_inode_load(vdisk, &minode, to_ino)))
        return error;

    if (!ufs_inode_isdir(&minode.dinode))
        return ENOTDIR;

    /* Lookup the parent. */
    do {
        struct lookup_parent_arg arg;
        /* Search parent dir */
        arg.parent_ino = 0;
        ufs_datablock_foreach(vdisk, &minode, lookup_parent_from_dir_subr, &arg);
        if ((parent = arg.parent_ino) == 0) {
            /* ufs_panic("%s ***parent missing", __func__); */
            return ENOENT;
        }

        /* Load parent dir */
        if ((error = ufs_inode_load(vdisk, &minode, parent)))
            return error;
        if (parent == from_ino) {
            /* ufs_panic("%s #%u is child dir of #%u", __func__, to_ino, from_ino); */
            return EINVAL;
        }
    } while (parent != UFS_ROOT_INODE);

    return 0;
}

__asm__("\n&FUNC    SETC 'lookup_parent_from_dir_subr'");
static INT32
lookup_parent_from_dir_subr(UFSVDISK *vdisk, void *ctx, UINT32 blk, UINT32 sz)
{
    struct lookup_parent_arg *arg = (struct lookup_parent_arg *)ctx;
    char    name[UFS_NAME_MAX + 1];
    void    *buf;
    INT32   ret = 0;
    UFSDIR  *dir;
    UINT32  i, n;

    buf = ufs_disk_get_buf(vdisk->disk);
    if (!buf) {
        ufs_panic("%s out of memory", __func__);
        ret = ENOMEM;
        goto quit;
    }

    if (ufs_vdisk_read(vdisk, buf, blk))
        goto quit;

    dir = (UFSDIR *)buf;
    n = sz / sizeof(*dir);

    if (!ufs_dirent_endian_convert(vdisk, dir, n)) {
        ret = UFS_ITERATOR_ERROR;
        goto quit;
    }

    for (i = 0; i < n; i++, dir++) {
        ufs_dirent_filename(name, dir->name);
        if (strncmp(dir->name, "..", UFS_NAME_MAX) != 0)
            continue;

        arg->parent_ino = dir->inode_number;
        ret = UFS_ITERATOR_BREAK;
        break;
    }

quit:
    if (buf) ufs_disk_free_buf(vdisk->disk, buf);
	return ret;
}
