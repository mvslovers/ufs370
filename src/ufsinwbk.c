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
/* Write back inode to disk. */
__asm__("\n&FUNC    SETC 'ufs_inode_writeback'");
INT32 ufs_inode_writeback(UFSMIN *minode)
{
    UFSVDISK    *vdisk  = minode->vdisk;
    INT32       error   = 0;
    void        *buf    = NULL;
    INT32       lockrc;
    UFSDIN      *di;
    UINT32      blk;
    UINT32      ofs;

    if (!vdisk) {
        ufs_panic("%s inode vdisk is NULL", __func__);
        return EIO;
    }

    if (ufs_inode_inquire_disk_location(vdisk, minode->inode_number, &blk, &ofs) != 0)
        return ENOENT;

    lockrc = lock(vdisk->disk, 0);
    if (lockrc==0) goto locked;
    if (lockrc==8) goto locked;
    ufs_panic("%s lock failure, rc=%d", __func__, lockrc);
    return ENOENT;

locked:
    if (!(buf = ufs_disk_get_buf(vdisk->disk))) {
        ufs_panic("%s out of memory", __func__);
        error = ENOMEM;
        goto done;
    }

    if (ufs_vdisk_read(vdisk, buf, blk)) {
        ufs_panic("%s read error", __func__);
        error = EIO;
        goto done;
    }

    di      = (UFSDIN *)buf;
    di[ofs] = minode->dinode;    /* simple structure copy */

    if (ufs_vdisk_write(vdisk, buf, blk)) {
        ufs_panic("%s write error", __func__);
		error = EIO;
		goto done;
    }

    /* clear the DIRTY flag */
    minode->flags &= ~MINODE_FLAG_DIRTY;

done:
    if (buf) ufs_disk_free_buf(vdisk->disk, buf);
    if (lockrc==0) unlock(vdisk->disk,0);

    return error;
}
