#include "ufs/file.h"
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

__asm__("\n&FUNC    SETC 'ufs_file_synlink'");
INT32 ufs_file_symlink(UFSMIN *p, const char *target)
{
    UFSVDISK *vdisk = p->vdisk;
    INT32   error   = 0;
    UINT32  len     = strlen(target) + 1;
    void    *buf    = NULL;
    UINT32  blk;

    if (len > UFSBSD_MAXSYMLINKLEN) {   /* limited target 512byte pathname */
        /* ufs_panic("%s too long pathname.", __func__); */
        return ENAMETOOLONG;
    }

    if ((error = ufs_datablock_expand(vdisk, p, len))) {
        goto quit;
    }

    blk = p->dinode.addr[0];   /* 1block only.  */
    buf = ufs_disk_get_buf(vdisk->disk);
    if (!buf) {
        ufs_panic("%s out of memory", __func__);
        error = ENOMEM;
        goto quit;
    }

    if (ufs_vdisk_read(vdisk, buf, blk)) {
        ufs_panic("%s read error", __func__);
        error = EIO;
        goto quit;
	}

    strncpy(buf, target, vdisk->disk->blksize);
    if (ufs_vdisk_write(vdisk, buf, blk)) {
        ufs_panic("%s write error", __func__);
        error = EIO;
        goto quit;
	}

    error = ufs_inode_writeback(p);

quit:
    if (buf) ufs_disk_free_buf(vdisk->disk, buf);

	return error;
}
