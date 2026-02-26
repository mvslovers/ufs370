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
__asm__("\n&FUNC    SETC 'ufs_freeblock_update'");
INT32 ufs_freeblock_update(UFSVDISK *vdisk, UINT32 blk)
{
	/* Assume superblock is locked by caller. */
	INT32   error   = 0;
    UFSDISK *disk   = vdisk->disk;
    UFSSB   *sb     = &disk->sb;
    UFSFB   *fb;
    void    *buf;

    buf = ufs_disk_get_buf(disk);
    if (!buf) {
        ufs_panic("%s out of memory", __func__);
        error = ENOMEM;
        goto quit;
    }

    /* Read next freeblock table from disk. */
    if (!ufs_datablock_number_sanity(vdisk, blk)) {
        ufs_panic("%s bad block %u", __func__, blk);
		error = EIO;
		goto quit;
    }

    if (ufs_vdisk_read(vdisk, buf, blk)) {
        ufs_panic("%s read failure for block %u", __func__, blk);
        error = EIO;
        goto quit;
    }

    /* Update in-core superblock freelist. */
    fb = (UFSFB *)buf;
	memcpy(sb->freeblock, fb->freeblock, sizeof(blk) * fb->nfreeblock);
	sb->nfreeblock = fb->nfreeblock;
	sb->modified = 1;

quit:
    if (buf) ufs_disk_free_buf(disk, buf);
	return error;
}
