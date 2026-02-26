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

static INT32 lookup_subr(UFSVDISK *vdisk, void *ctx, UINT32 blk, UINT32 sz);

__asm__("\n&FUNC    SETC 'ufs_file_lookup_by_name'");
INT32 ufs_file_lookup_by_name(UFSMIN *parent_dir, const char *name, UINT32 *ino)
{
    UFSVDISK    *vdisk      = parent_dir->vdisk;
    char        filename[UFS_NAME_MAX + 1];
    char        *q;
    INT32       error;
    UINT32      len;
    UFSLKARG    lookup_arg = {0};

    // wtof("%s: enter name=\"%s\"", __func__, name);

    error = ufs_inode_isdir(&parent_dir->dinode);
    if (!error) {
		wtof("%s: ufs_inode_isdir() rc=%d", __func__, error);
		error = ENOTDIR;
		goto quit;
	}

    if ((q = strchr(name, '/'))) {
        /* Zap following path. */
        len = MIN(UFS_NAME_MAX, q - name);
        memcpy(filename, name, len);
        filename[len] = '\0';	/* '/' -> '\0' */
    }
    else {
        ufs_dirent_filename(filename, name);
	}

    // wtof("%s: filename=\"%s\" dir->inode_number=%d", __func__, filename, parent_dir->inode_number);

    lookup_arg.name         = filename;
    lookup_arg.inode_number = 0;
    error = ufs_datablock_foreach(vdisk, parent_dir, lookup_subr, &lookup_arg);
    if (error != UFS_ITERATOR_BREAK) {
        // wtof("%s: not found.", __func__);
        error = ENOENT;
        goto quit;
    }

    *ino = lookup_arg.inode_number;
    
    // wtof("%s: done. ino=%d", __func__, *ino);
	error = 0;

quit:
	// wtof("%s: exit rc=%d %s", __func__, error, error ? strerror(error) : "SUCCESS");
    return error;
}

__asm__("\n&FUNC    SETC 'lookup_subr'");
static INT32
lookup_subr(UFSVDISK *vdisk, void *ctx, UINT32 blk, UINT32 sz)
{
    INT32       ret     = 0;
    UFSLKARG    *p      = (UFSLKARG *)ctx;
    const char  *name   = p->name;
    void        *buf    = NULL;
    UFSDIR      *dir;
    UINT32      i, n;

#if 0
    wtof("   %s: enter blk=%u, sz=%u", __func__, blk, sz);
    wtof("   %s: lookup_arg.name=\"%s\"", __func__, p->name);
    wtof("   %s: lookup_arg.inode_number=%u", __func__, p->inode_number);
#endif

    buf = ufs_disk_get_buf(vdisk->disk);
    if (!buf) {
		wtof("   %s: ufs_disk_get_buf(%08X) returned NULL", __func__, vdisk->disk);
        ret = ENOMEM;
        goto quit;
    }

    if (ufs_vdisk_read(vdisk, buf, blk)) {
		wtof("   %s: ufs_vdisk_read(%08X,%08X,%u) returned NULL", 
			__func__, vdisk, buf, blk);
        ret = EIO;
        goto quit;
    }

    dir = (UFSDIR *)buf;
    n   = sz / sizeof(*dir);

    for (i = 0; i < n; i++, dir++) {
		// wtof("   %s: dir->inode_number=%u dir->name=\"%s\"", 
		// 	__func__, dir->inode_number, dir->name);

        if (dir->inode_number < 1) {
            ufs_panic("%s *** bad inode #%u ***", __func__, dir->inode_number);
            wtodumpf(dir, sizeof(UFSDIR), "%s BLK=%u, DIR=%u", __func__, blk, i);
            continue;
        }

        if (strncmp((const char *)dir->name, name, UFS_NAME_MAX) == 0) {
            p->inode_number     = dir->inode_number;
            ret = UFS_ITERATOR_BREAK; /* found */
            break;
        }
    }

quit:
    if (buf) ufs_disk_free_buf(vdisk->disk, buf);
#if 0
	wtof("   %s: exit ret=%d %s", 
		__func__, ret, ret==UFS_ITERATOR_BREAK ? "UFS_ITERATOR_BREAK" : strerror(ret));
#endif
	return ret;
}
