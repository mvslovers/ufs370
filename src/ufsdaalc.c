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
__asm__("\n&FUNC    SETC 'ufs_datablock_allocate'");
INT32 ufs_datablock_allocate(UFSVDISK *vdisk, UINT32 *block_number)
{
    INT32   error   = 0;
    UFSSB   *sb     = &vdisk->disk->sb;
    INT32   lockrc;
    UINT32  blk;
	void    *buf;
	char    caller[64] = "";

	*block_number = 0;

    lockrc = lock(vdisk->disk,0);
    if (lockrc==0) goto locked;
    if (lockrc==8) goto locked;
    ufs_panic("%s lock failed, rc=%d", __func__, lockrc);
    return EPERM;

locked:
    for(;;) {
        if (!sb->total_freeblock) {
            ufs_panic("%s free block exhausted!!!", __func__);
            error = ENOSPC;
            break;
        }

        /* Get free block from superblock cache. */
        blk = sb->freeblock[--sb->nfreeblock];
        /* wtof("%s assigned block %u", __func__, blk); */
        sb->total_freeblock--;
        sb->modified = 1;

        /* If nfreeblock is zero, this block is next free block link. */
        if (sb->nfreeblock == 0) {
            /* replenish the free block cache in the super block */
            /* wtof("%s sb->nfreeblock = %d", __func__, sb->nfreeblock); */
            error = ufs_freeblock_update(vdisk, blk);
            /* wtof("%s ufs_freeblock_update(vdisk,%u) rc=%d", __func__, blk, error); */
            /* wtof("%s sb->nfreeblock = %d", __func__, sb->nfreeblock); */
            if (error) {
                ufs_panic("%s no freeblock!!!", __func__);
            }
        }

        /* if this block is valid then we're done */
        if (ufs_datablock_number_sanity(vdisk, blk)) break;
    }

    if (lockrc==0) unlock(vdisk->disk, 0);
    if (error) return error;

    /* ufs_panic("%s freeblock %u", __func__, blk); */

	/* Zero clear datablock on disk/cache */
	buf = ufs_disk_get_buf(vdisk->disk);
	if (!buf) {
        ufs_panic("%s out of memory", __func__);
        error = ENOMEM;
        goto quit;
	}

	/* read the allocated data block */
	if (ufs_vdisk_read(vdisk, buf, blk)) {
        ufs_panic("%s read error for block %u", __func__, blk);
        error = EIO;
        goto quit;
    }

    /* clear the allocated data block */
    memset(buf, 0, vdisk->disk->blksize);
    if (ufs_vdisk_write(vdisk, buf, blk)) {
        ufs_panic("%s write error for block %u", __func__, blk);
		error = EIO;
		goto quit;
    }

    /* success, return block number */
    *block_number = blk;
#if 0
    __caller(caller);
    wtof("%s allocated block %u for caller %s", __func__, blk, caller);
#endif
quit:
    if (buf) ufs_disk_free_buf(vdisk->disk, buf);
    return error;
}
