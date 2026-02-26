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

__asm__("\n&FUNC    SETC 'ufs_file_deallocate'");
INT32 ufs_file_deallocate(UFSMIN *parent_dir, const char *name)
{
    UFSVDISK *vdisk = parent_dir->vdisk;
    INT32   error   = 0;
    UINT32  ino;
    UINT32  filesize;
    UFSMIN  *minode = NULL;
    char    filename[UFS_NAME_MAX + 1];

	// wtof("%s: enter parent_dir=%08X name=\"%s\"", __func__, parent_dir, name);
	
    error = ufs_file_lookup_by_name(parent_dir, name, &ino);
    if (error) {
		// wtof("%s: ufs_file_lookup_by_name(\"%s\") rc=%d", __func__, name, error);
		goto quit;
	}

    minode = ufs_inode_get(vdisk, ino);
    if (!minode) {
		wtof("%s: ufs_inode_get(%08X, %u) returned NULL", __func__, vdisk, ino);
		error = ENOENT;
		goto quit;
	}

#if 1
    /* check for mount point on this inode */
    if (minode->mounted_vdisk) {
		// wtof("%s:- - - - - - - - - - - - - - - - - - - - - - - - - - -", __func__);
		// wtof("%s: if (minode->mounted_vdisk) ...", __func__);
		// wtodumpf(minode->vdisk, sizeof(UFSVDISK), "%s: minode->vdisk", __func__);
        /* follow mount point to mounted disk */
        vdisk = minode->mounted_vdisk;
        /* release the current inode */
        ufs_inode_rel(minode);
        /* get the root inode on the mounted disk */
        minode = ufs_inode_get(vdisk, UFS_ROOT_INODE);
        if (!minode) {
			error = ENOENT;
			goto quit;
		}
		// wtof("%s: minode->mounted_vdisk", __func__);
		// wtodumpf(minode->vdisk, sizeof(UFSVDISK), "%s: minode->vdisk", __func__);
		// wtof("%s:- - - - - - - - - - - - - - - - - - - - - - - - - - -", __func__);
    }
#endif

    if (ufs_inode_isdir(&minode->dinode)) {
		// wtof("%s: minode is directory", __func__);
        ufs_dirent_filename(filename, name);
        /* Check parent */
        if (strcmp(filename, "..") == 0) {
			// wtof("%s: filename is '..'", __func__);
            error = EINVAL;
            goto quit;
        }

        /* Check self */
        if (strcmp(filename, ".") == 0) {
			// wtof("%s: filename is '.'", __func__);
            error = EINVAL;
            goto quit;
        }

        /* Check empty */
        filesize = ufs_inode_filesize(&minode->dinode);
        // wtof("%s: filesize=%u", __func__, filesize);
        if (filesize > 0) {
            if (filesize != sizeof(UFSDIR) * 2) {  /*"." + ".."*/
				// wtof("%s: filesize != %u ENOTEMPTY", __func__, sizeof(UFSDIR)*2);
                error = ENOTEMPTY;
                goto quit;
            }

            error = ufs_datablock_size_change(vdisk, 0, minode);
            // wtof("%s: ufs_datablock_size_change(%08X, 0, %08X) rc=%d", __func__, vdisk, minode, error);
            if (error) {
                goto quit;
            }
        }
        minode->dinode.nlink = 0;	/* remove this. */
    }
    else {
        /* Decrement reference count. */
        if (minode->dinode.nlink > 0) --minode->dinode.nlink;	/* regular file. */
    }

    ufs_inode_writeback(minode);

    error = ufs_directory_remove_entry(vdisk, parent_dir, name);
    if (error) {
        // wtof("%s ufs_directory_remove_entry(%08X, %08X, \"%s\") rc = %d", __func__, vdisk, parent_dir, name, error);
        goto quit;
    }

quit:
    if (minode) ufs_inode_rel(minode);
	// wtof("%s: exit rc=%d %s", __func__, error, error ? strerror(error) : "SUCCESS");
	
    return error;
}
