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
__asm__("\n&FUNC    SETC 'ufs_inode_allocate'");
INT32 ufs_inode_allocate(UFSVDISK *vdisk, UINT32 *inode)
{
    UFSSB   *sb             = &vdisk->disk->sb;
    INT32   error           = ENOSPC;
    INT32   lockrc;
    UINT32  inode_number;
    char    caller[64] = "";

    *inode = 0;

    if (sb->readonly) {
        /* can't update file system if we're read only */
        error = EROFS;  /* read only file system */
        goto quit;
    }

    lockrc = lock(vdisk->disk,0);
    if (lockrc==0) goto locked;
    if (lockrc==8) goto locked;
    ufs_panic("%s lock failed, rc=%d", __func__, lockrc);
    return error;

locked:
    if (sb->total_freeinode == 0) {
        ufs_panic("%s inode exhausted!(1)", __func__);
        goto errexit;
    }

    /* If there is no free inode cache, update it. */
    if (sb->nfreeinode <= 0) {
        /* wtof("%s sb->nfreeinode = %d", __func__, sb->nfreeinode); */
        error = ufs_freeinode_update(vdisk);
        /* wtof("%s ufs_freeinode_update(vdisk) rc=%d", __func__, error); */
        /* wtof("%s sb->nfreeinode = %d", __func__, sb->nfreeinode); */
        if (error) {
            ufs_panic("%s inode exhausted!(2)\n", __func__);
            goto errexit;
        }
    }

    /* Get inode from superblock cache. */
    /* KDASSERT(sb->nfreeinode <= V7FS_MAX_FREEINODE); */
    inode_number = sb->freeinode[--sb->nfreeinode];
    sb->total_freeinode--;
    sb->modified = 1;

    if ((error = ufs_inode_number_sanity(vdisk, inode_number))) {
        wtof("%s new inode#%d %d %d\n", __func__, inode_number,
             sb->nfreeinode, sb->total_freeinode);
        ufs_panic("%s free inode list corupt\n", __func__);
        goto errexit;
    }

    *inode = inode_number;
#if 0
    __caller(caller);
    wtof("%s allocated inode %u for caller %s", __func__, inode_number, caller);
#endif
errexit:
    if (lockrc==0) unlock(vdisk->disk,0);

quit:
    return error;
}
