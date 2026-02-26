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
__asm__("\n&FUNC    SETC 'ufs_datablock_deallocate'");
INT32 ufs_datablock_deallocate(UFSVDISK *vdisk, UINT32 blk)
{
    INT32   error   = 0;
    INT32   lockrc  = 0;
    UFSSB   *sb     = &vdisk->disk->sb;
    void    *buf    = NULL;
    UFSFB   *fb;
    INT32   i;

    if (!ufs_datablock_number_sanity(vdisk, blk))
        return EIO;

    /* Add to in-core freelist. */
    lockrc = lock(vdisk->disk,0);
    if (lockrc==0) goto locked;
    if (lockrc==8) goto locked;
    ufs_panic("%s lock failed, rc=%d", __func__, lockrc);
    return EPERM;

locked:
	if (sb->nfreeblock < UFS_MAX_FREEBLOCK) {
        sb->freeblock[sb->nfreeblock++] = blk;
        sb->total_freeblock++;
        sb->modified = 1;
        /* ufs_panic("%s n_freeblock=%d\n", __func__, sb->total_freeblock); */
        goto quit;
    }

    /* No space to push. */
    buf = ufs_disk_get_buf(vdisk->disk);
    if (!buf) {
        ufs_panic("%s out of memory", __func__);
        error = ENOMEM;
        goto quit;
    }

    /* Make this block a freeblock list and move current cache to block */
	if (ufs_vdisk_read(vdisk, buf, blk)) {
        ufs_panic("%s read error", __func__);
        error = EIO;
        goto quit;
    }

    memset(buf, 0, vdisk->disk->blksize);
    fb = (UFSFB *)buf;
    fb->nfreeblock = UFS_MAX_FREEBLOCK;
    for (i = 0; i < UFS_MAX_FREEBLOCK; i++)
        fb->freeblock[i] = sb->freeblock[i];

    if (ufs_vdisk_write(vdisk, buf, blk)) {
        ufs_panic("%s write error", __func__);
        error = EIO;    /* Fatal */
        goto quit;
    }

    /* on next allocate, this block is used as datablock, */
    /* and the swap outed freeblock list will be restored. */
    memset(sb->freeblock, 0, UFS_MAX_FREEBLOCK);
    sb->freeblock[0]    = blk;
    sb->nfreeblock      = 1;
    sb->modified        = 1;
    sb->total_freeblock++;
    /* ufs_panic("%s n_freeblock=%d\n", __func__, sb->total_freeblock); */

quit:
    if (buf) ufs_disk_free_buf(vdisk->disk, buf);
    if (lockrc==0) unlock(vdisk->disk, 0);

    return error;
}
