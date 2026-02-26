#include "ufs/data.h"
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
__asm__("\n&FUNC    SETC 'ufs_datablock_expand'");
INT32 ufs_datablock_expand(UFSVDISK *vdisk, UFSMIN *minode, UINT32 sz)
{
    UFSDISK *disk           = vdisk->disk;
    UFSSB   *sb             = &disk->sb;
    UFSDIN  *dinode         = &minode->dinode;
    UINT32  old_filesize    = dinode->filesize;
    UINT32  new_filesize    = old_filesize + sz;
    UFSADRM oldmap, newmap;
    UINT32  blk, idxblk;
	INT32   error;
	UINT32  old_nblk        = UFS_ROUND_BSIZE(old_filesize,disk->blksize) >> sb->blksize_shift;
    UINT32  new_nblk        = UFS_ROUND_BSIZE(new_filesize,disk->blksize) >> sb->blksize_shift;
    UINT32  i;
    UFSMIN  backup;
    UINT32  required_blk;

    if (old_nblk == new_nblk) {
        dinode->filesize += sz;
		ufs_inode_writeback(minode);
		return 0; /* no need to expand. */
	}

    backup          = *minode;
    required_blk    = new_nblk - old_nblk;

    /* ufs_panic("%s %u->%u, required block=%u", __func__, old_filesize, new_filesize, required_blk); */

    ufs_datablock_addr(vdisk, old_filesize, &oldmap);
    for (i = 0; i < required_blk; i++) {
        ufs_datablock_addr(vdisk, old_filesize + (i+1) * disk->blksize, &newmap);
        /* ufs_daddr_map_dump(&oldmap); */
        /* ufs_daddr_map_dump(&newmap); */

        if (oldmap.level != newmap.level) {
            /* Allocate index area */
            if ((error = ufs_datablock_allocate(vdisk, &idxblk)))
                return error;

            switch (newmap.level) {
            case 1:
                /* ufs_panic("%s 0->1", __func__); */
                dinode->addr[UFS_NADDR_INDEX1] = idxblk;
                blk = ufs_datablock_add_leaf(vdisk, idxblk, 0);
                break;
            case 2:
                /* ufs_panic("%s 1->2", __func__); */
                dinode->addr[UFS_NADDR_INDEX2] = idxblk;
                blk = ufs_datablock_add_leaf(vdisk, ufs_datablock_add_leaf(vdisk, idxblk, 0), 0);
                break;
            case 3:
                /* ufs_panic("%s 2->3", __func__); */
                dinode->addr[UFS_NADDR_INDEX3] = idxblk;
                blk = ufs_datablock_add_leaf(vdisk, ufs_datablock_add_leaf(vdisk, ufs_datablock_add_leaf(vdisk, idxblk, 0), 0), 0);
                break;
            }
        }
        else {
            switch (newmap.level) {
            case 0:
                if ((error = ufs_datablock_allocate(vdisk, &blk)))
                    return error;
                dinode->addr[newmap.index[0]] = blk;
                /* ufs_panic("%s direct index %d = blk%d", __func__, newmap.index[0], blk); */
                break;
            case 1:
                idxblk = dinode->addr[UFS_NADDR_INDEX1];
                blk = ufs_datablock_add_leaf(vdisk, idxblk, newmap.index[0]);
                break;
            case 2:
                idxblk = dinode->addr[UFS_NADDR_INDEX2];
                if (oldmap.index[0] != newmap.index[0]) {
                    ufs_datablock_add_leaf(vdisk, idxblk, newmap.index[0]);
                }
                blk = ufs_datablock_add_leaf(vdisk, ufs_datablock_link(vdisk,idxblk, newmap.index[0]), newmap.index[1]);
                break;
            case 3:
                idxblk = dinode->addr[UFS_NADDR_INDEX3];

                if (oldmap.index[0] != newmap.index[0]) {
                    ufs_datablock_add_leaf(vdisk, idxblk, newmap.index[0]);
                }

                if (oldmap.index[1] != newmap.index[1]) {
                    ufs_datablock_add_leaf(vdisk, ufs_datablock_link(vdisk, idxblk, newmap.index[0]), newmap.index[1]);
                }

                blk = ufs_datablock_add_leaf(vdisk, ufs_datablock_link(vdisk, ufs_datablock_link(vdisk, idxblk, newmap.index[0]), newmap.index[1]), newmap.index[2]);
                break;
            }
        }

        if (!blk) {
            *minode = backup; /* structure copy; */
            return ENOSPC;
        }
        oldmap = newmap;
    }

	dinode->filesize += sz;
    ufs_inode_writeback(minode);

    return 0;
}
