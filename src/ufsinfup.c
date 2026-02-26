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
/* Fill free inode to superblock cache. */
__asm__("\n&FUNC    SETC 'ufs_freeinode_update'");
INT32 ufs_freeinode_update(UFSVDISK *vdisk)
{
    INT32   rc          = 0;
    UFSSB   *sb         = &vdisk->disk->sb;
    INT32   lockrc;
    UINT32  i, j, k;
    UINT32  inode;
    UFSDIN  *di;
    void    *buf;

    if (sb->readonly) {
        /* can't update super block if we're read only */
        rc = EROFS;     /* read only file system */
        goto quit;
    }

    lockrc = lock(vdisk->disk,0);
    if (lockrc==0) goto locked;
    if (lockrc==8) goto locked;
    ufs_panic("%s lock failed, rc=%d", __func__, lockrc);
    return;

locked:
    /* Loop over all inode list. */
    k       = sb->nfreeinode;   /* will be 0 if cache is empty */
    inode   = 1;                /* inode start from 1 */
    for (i = sb->ilist_sector; i < sb->datablock_start_sector; i++) {
		if (!(buf = ufs_disk_get_buf(vdisk->disk))) {
            ufs_panic("%s out of memory.", __func__);
            rc = ENOMEM;
            break;
		}

		if (ufs_vdisk_read(vdisk, buf, i)) {
            /* should never happen, but just in case it does */
            ufs_panic("%s block %u I/O error.", __func__, i);
			inode += sb->inodes_per_block;
			continue;
		}

        di = (UFSDIN *)buf;

        for (j = 0; (j < sb->inodes_per_block) && (k < UFS_MAX_FREEINODE); j++, di++, inode++) {
            if (ufs_inode_allocated(di)) continue;
            sb->freeinode[k++] = inode;
            sb->modified = 1;
        }
		ufs_disk_free_buf(vdisk->disk, buf);
    }

    /* since the super block is not otherwise cached, we need to write it to disk */
    /* when we've modified it. */
    if (sb->modified) {
        sb->modified = 0;       /* reset the super block modified flag */
        sb->nfreeinode = k;     /* update count of free inodes in super block cache */

        if (ufs_disk_write_super(vdisk->disk, sb)) {
            ufs_panic("%s super block I/O error", __func__);
            sb->modified = 1;   /* indicate super block not written to disk */
            rc = EIO;           /* return I/O error to caller */
        }
    }

    if (lockrc==0) unlock(vdisk->disk,0);

quit:
    return rc;
}
