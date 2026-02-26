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

static INT32 lookup_by_number_subr(UFSVDISK *vdisk, void *ctx, UINT32 blk, UINT32 sz);

__asm__("\n&FUNC    SETC 'ufs_file_lookup_by_number'");
BOOL ufs_file_lookup_by_number(UFSMIN *parent_dir, UINT32 ino, char *buf)
{
    UFSVDISK *vdisk = parent_dir->vdisk;
    INT32       ret;
    UFSLKARG    arg = {0};

    arg.inode_number = ino;
    arg.buf          = buf;

    ret = ufs_datablock_foreach(vdisk, parent_dir, lookup_by_number_subr, &arg);

    return (ret == UFS_ITERATOR_BREAK);
}

__asm__("\n&FUNC    SETC 'lookup_by_number_subr'");
static INT32
lookup_by_number_subr(UFSVDISK *vdisk, void *ctx, UINT32 blk, UINT32 sz)
{
    INT32       ret     = 0;
    UFSLKARG    *p      = (UFSLKARG *)ctx;
    UFSDIR      *dir;
    void        *buf;
    UINT32      i, n;

    buf = ufs_disk_get_buf(vdisk->disk);
    if (!buf) {
        ufs_panic("%s out of memory", __func__);
        ret = ENOMEM;
        goto quit;
    }

    if (ufs_vdisk_read(vdisk, buf, blk)) {
        ufs_panic("%s read error", __func__);
        ret = EIO;
        goto quit;
    }

    dir = (UFSDIR *)buf;
    n = sz / sizeof(*dir);

	ufs_dirent_endian_convert(vdisk, dir, n);

    for (i = 0; i < n; i++, dir++) {
        if (dir->inode_number == p->inode_number) {
            if (p->buf)
                ufs_dirent_filename(p->buf, dir->name);
            ret = UFS_ITERATOR_BREAK;
            break;
        }
    }

quit:
    if (buf) ufs_disk_free_buf(vdisk->disk, buf);

    return ret;
}
