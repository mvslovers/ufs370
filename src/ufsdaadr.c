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
__asm__("\n&FUNC    SETC 'ufs_datablock_addr'");
INT32 ufs_datablock_addr(UFSVDISK *vdisk, UINT32 sz, UFSADRM *map)
{
    UFSDISK     *disk       = vdisk->disk;
    UFSSB       *sb         = &disk->sb;
    UINT32      blksize     = (UINT32)disk->blksize;
    UINT32      blkshift    = sb->blksize_shift;
    UINT32      nidx        = UFS_DADDR_PER_BLOCK(blksize);
    UINT32      direct_sz   = (UFS_NADDR_DIRECT * blksize);
    UINT32      idx1_sz     = (nidx * blksize);
    UINT32      idx2_sz     = (nidx * nidx * blksize);

#define	ROUND(x, a)	((((x) + ((a) - 1)) & ~((a) - 1)))

    if (!sz) {
        map->level = 0;
        map->index[0] = 0;
        return 0;
    }

    sz = UFS_ROUND_BSIZE(sz,blksize);

    /* Direct */
    if (sz <= direct_sz) {
        map->level = 0;
        map->index[0] = (sz >> blkshift) - 1;
        return 0;
    }

    /* Index 1 */
    sz -= direct_sz;

    if (sz <= idx1_sz) {
        map->level = 1;
        map->index[0] = (sz >> blkshift) - 1;
        return 0;
    }

    sz -= idx1_sz;

    /* Index 2 */
    if (sz <= idx2_sz) {
        map->level = 2;
        map->index[0] = ROUND(sz, idx1_sz) / idx1_sz - 1;
        map->index[1] = ((sz - (map->index[0] * idx1_sz)) >> blkshift) - 1;
        return 0;
    }

    sz -= idx2_sz;

    /* Index 3 */
    map->level = 3;
    map->index[0] = ROUND(sz, idx2_sz) / idx2_sz - 1;
    sz -= map->index[0] * idx2_sz;
    map->index[1] = ROUND(sz, idx1_sz) / idx1_sz - 1;
    sz -= map->index[1] * idx1_sz;
    map->index[2] = (sz >> blkshift) - 1;

    return map->index[2] >= nidx ? ENOSPC : 0;
}
