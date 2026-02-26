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
__asm__("\n&FUNC    SETC 'ufs_datablock_unlink'");
UINT32 ufs_datablock_unlink(UFSVDISK *vdisk, UINT32 idxblk, int n)
{
    UINT32  *daddr_list;
    UINT32  blk     = 0;
    void    *buf;

    buf = ufs_disk_get_buf(vdisk->disk);
    if (!buf) {
        ufs_panic("%s out of memory", __func__);
        goto quit;
    }

    if (ufs_vdisk_read(vdisk, buf, idxblk)) {
        ufs_panic("%s read error block %u", __func__, idxblk);
        goto quit;
    }

	daddr_list = (UINT32 *)buf;
    blk = daddr_list[n];
    daddr_list[n] = 0;

    if (ufs_vdisk_write(vdisk, buf, idxblk)) {
        ufs_panic("%s write error block %u", __func__, idxblk);
        blk = 0;
    }

quit:
    if (buf) ufs_disk_free_buf(vdisk->disk, buf);

    return blk; /* unlinked block. */
}
