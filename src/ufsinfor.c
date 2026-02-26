#include "ufs/inode.h"
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
__asm__("\n&FUNC    SETC 'ufs_ilist_foreach'");
/* Loop over all inode in ilist. */
INT32 ufs_ilist_foreach(UFSVDISK *vdisk,
                        INT32 (*func)(UFSVDISK *, void *, UFSMIN *, UINT32), void *ctx)
{
    INT32   ret     = EIO;
    UFSSB   *sb     = &vdisk->disk->sb;
    void    *buf    = NULL;
    UINT32  i, j, k;
    UFSDIN  *di;
    UFSMIN  inode;

    if (!(buf = ufs_disk_get_buf(vdisk->disk))) {
        ufs_panic("%s out of memory", __func__);
        return ENOMEM;
    }

    /* Loop over ilist. */
    for (k = 1, i = sb->ilist_sector; i < sb->datablock_start_sector; i++) {
		if (ufs_vdisk_read(vdisk, buf, i)) {
            ufs_panic("%s block %u I/O error", i);
            k += sb->inodes_per_block;
			continue;
		}

        di = (UFSDIN *)buf;
        for (j = 0; j < sb->inodes_per_block; j++, k++) {
            ufs_inode_setup_memory_image(vdisk, &inode, di + j);
            inode.inode_number = k;
            if ((ret = func(vdisk, ctx, &inode, k))) break;
        }

        if (ret) break;
    }

    if (buf) ufs_disk_free_buf(vdisk->disk, buf);
    return ret;
}
