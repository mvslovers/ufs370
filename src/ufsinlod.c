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
__asm__("\n&FUNC    SETC 'ufs_inode_load'");
INT32 ufs_inode_load(UFSVDISK *vdisk, UFSMIN *minode, UINT32 inode)
{
    INT32   lockrc;
    UINT32  blk, ofs;
    UFSDIN  *di;
    void *buf;

    if (ufs_inode_inquire_disk_location(vdisk, inode, &blk, &ofs) != 0)
		return ENOENT;

    lockrc = lock(vdisk->disk, 0);
    if (lockrc==0) goto locked;
    if (lockrc==8) goto locked;
    ufs_panic("%s lock failure, rc=%d", __func__, lockrc);
    return ENOENT;

locked:
    buf = ufs_disk_get_buf(vdisk->disk);
    if (!buf) {
        ufs_panic("%s out of memory", __func__);
        goto error;
    }

	if (ufs_vdisk_read(vdisk, buf, blk)) {
        ufs_panic("%s ufs_vdisk_read error", __func__);
        goto error;
	}

    if (lockrc==0) unlock(vdisk->disk,0);

    di = (UFSDIN *)buf;

    ufs_inode_setup_memory_image(vdisk, minode, di + ofs);
	minode->inode_number = inode;
	if (inode==UFS_ROOT_INODE)  minode->type |= MINODE_TYPE_FSROOT;

	if (buf) ufs_disk_free_buf(vdisk->disk, buf);
	return 0;

error:
    if (lockrc==0) unlock(vdisk->disk,0);
	if (buf) ufs_disk_free_buf(vdisk->disk, buf);
    return EIO;
}
