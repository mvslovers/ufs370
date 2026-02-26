#include "ufs/dir.h"
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

__asm__("\n&FUNC    SETC 'ufs_directory_add_entry'");
INT32 ufs_directory_add_entry(UFSVDISK *vdisk, UFSMIN *parent_dir, UINT32 ino, const char *srcname)
{
    UFSSB   *sb     = &vdisk->disk->sb;
    UFSMIN  *minode = NULL;
    void    *buf    = NULL;
    INT32   error   = 0;
    UFSDIR  *dir;
    UINT32  blk;
    UINT32  sz;
    INT32   n;
    char    filename[UFS_NAME_MAX + 1];

    /* Truncate filename. */
    ufs_dirent_filename(filename, srcname);
    /* ufs_panic("%s %s(%s) %u", __func__, filename, srcname, ino); */

    /* Target inode */
    minode = ufs_inode_get(vdisk, ino);
    if (!minode) {
        error = ENOENT;
        goto quit;
    }

    /* Expand datablock. */
    if ((error = ufs_datablock_expand(vdisk, parent_dir, sizeof(*dir))))
        goto quit;

    /* Read last entry. */
    if (!(blk = ufs_datablock_last(parent_dir, ufs_inode_filesize(&parent_dir->dinode)))) {
        error = EIO;
        goto quit;
    }

    /* Load dirent block. This vnode(parent dir) is locked by VFS layer. */
    buf = ufs_disk_get_buf(vdisk->disk);
    if (!buf) {
        error = ENOMEM;
        goto quit;
    }

    if (ufs_vdisk_read(vdisk, buf, blk)) {
        error = EIO;
        goto quit;
    }

    sz = ufs_inode_filesize(&parent_dir->dinode);
    sz = UFS_RESIDUE_BSIZE(sz,sb->blksize_shift);	/* last block payload. */

#if 0 /* this bit of code was to fix bad directory names during testing */
	dir = (UFSDIR *)buf;
	for(n=0; n < sz; n++) {
		char *p;
		int	 found = 0;
		
		while(p=strchr(dir[n].name, '/')) {
			*p = '@';
			found++;
		}
		if (found) {
			if (ufs_vdisk_write(vdisk, buf, blk)) {
				error = EIO;
				goto quit;
			}
		}
	}
#endif

    n = sz / sizeof(*dir) - 1;

    /* Add dirent. */
    dir = (UFSDIR *)buf;
    dir[n].inode_number = ino;
    memcpy((char *)dir[n].name, filename, UFS_NAME_MAX);

    /* Write back datablock */
    if (ufs_vdisk_write(vdisk, buf, blk)) {
        error = EIO;
        goto quit;
    }

    if (ufs_inode_isdir(&minode->dinode)) {
        parent_dir->dinode.nlink++;
        ufs_inode_writeback(parent_dir);
    }

    /* ufs_panic("%s done. (dirent size=%u byte)", __func__, parent_dir->filesize); */

quit:
    if (buf) ufs_disk_free_buf(vdisk->disk, buf);
    if (minode) ufs_inode_rel(minode);
    return error;
}
