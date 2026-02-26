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

static INT32 replace_subr(UFSVDISK *vdisk, void *ctx, UINT32 blk, UINT32 sz);

__asm__("\n&FUNC    SETC 'ufs_directory_replace_entry'");
INT32 ufs_directory_replace_entry(UFSVDISK *vdisk,  UFSMIN *self_dir, const char *name, UINT32 ino)
{
    INT32       error       = 0;
    UFSLKARG    lookup_arg  = {0};

    /* Search entry that replaced. replace it to new inode number. */
    lookup_arg.name         = name;
    lookup_arg.inode_number = ino;
    if ((error = ufs_datablock_foreach(vdisk, self_dir, replace_subr, &lookup_arg)) != UFS_ITERATOR_BREAK)
        return ENOENT;

    return 0;
}

__asm__("\n&FUNC    SETC 'replace_subr'");
static INT32
replace_subr(UFSVDISK *vdisk, void *ctx, UINT32 blk, UINT32 sz)
{
    INT32   ret = 0;
    UFSLKARG *p = (UFSLKARG *)ctx;
    UFSDIR  *dir;
    void    *buf;
    UINT32  i, n;

    /* ufs_panic("%s match start blk=%u", __func__, blk); */
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
    n = sz / sizeof(*dir);

    for (i = 0; i < n; i++, dir++) { /*disk endian */
        if (strncmp(p->name, (const char *)dir->name, UFS_NAME_MAX) == 0) {
            /* Replace inode# */
            dir->inode_number = p->inode_number;
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
