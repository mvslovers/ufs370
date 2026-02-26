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
__asm__("\n&FUNC    SETC 'ufs_datablock_last'");
UINT32 ufs_datablock_last(UFSMIN *minode, UINT32 offset)
{
    UFSVDISK    *vdisk  = minode->vdisk;
    UFSDIN      *dinode = &minode->dinode;
    UINT32      *addr   = dinode->addr;
    UINT32      blk     = 0;
    UFSADRM     map;

    /* Inquire last data block location. */
    if (ufs_datablock_addr(vdisk, offset, &map) != 0)
        return 0;

    switch (map.level) {
    case 0: /*Direct */
        blk = dinode->addr[map.index[0]];
        break;
    case 1: /*Index1 */
        blk = ufs_datablock_link(vdisk, addr[UFS_NADDR_INDEX1], map.index[0]);
        break;
    case 2: /*Index2 */
        blk = ufs_datablock_link(vdisk, ufs_datablock_link(vdisk, addr[UFS_NADDR_INDEX2], map.index[0]), map.index[1]);
        break;
    case 3: /*Index3 */
        blk = ufs_datablock_link(vdisk, ufs_datablock_link(vdisk, ufs_datablock_link(vdisk, addr[UFS_NADDR_INDEX3], map.index[0]), map.index[1]), map.index[2]);
        break;
    }

    return blk;
}
