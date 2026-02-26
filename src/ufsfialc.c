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

__asm__("\n&FUNC    SETC 'ufs_file_allocate'");
INT32 ufs_file_allocate(UFSMIN *parent_dir, const char *srcname, UFSATTR *attr, UINT32 *ino)
{
    UFSVDISK *vdisk = parent_dir->vdisk;
    UFSMIN  *minode;
    UFSDIN  *dinode;
    char    filename[UFS_NAME_MAX + 1];
    UFSDIR  *dir;
    INT32   error;
    UINT32  blk;
    void    *buf;

    /* Truncate filename. */
    ufs_dirent_filename(filename, srcname);
    /* ufs_panic("%s "%s(%s)", __func__, filename, srcname); */

    /* Check filename. */
    if (ufs_file_lookup_by_name(parent_dir, filename, ino) == 0) {
        /* ufs_panic("%s %s exists", __func__, filename); */
        return EEXIST;
    }

    /* Get new inode. */
    if ((error = ufs_inode_allocate(vdisk, ino)))
        return error;

    /* Set initial attribute. */
    minode = ufs_inode_get(vdisk, *ino);
    if (!minode) return ENOMEM;
    dinode = &minode->dinode;

    dinode->mode  = attr->mode;
    dinode->codepage = attr->codepage;
    if (attr->owner && attr->owner[0]) {
        strcpyp(dinode->owner, sizeof(dinode->owner)-1, attr->owner, 0);
        dinode->owner[sizeof(dinode->owner)-1] = 0;
    }
    if (attr->group && attr->group[0]) {
        strcpyp(dinode->group, sizeof(dinode->group)-1, attr->group, 0);
        dinode->group[sizeof(dinode->group)-1] = 0;
    }
    if (attr->ctime.v1.seconds > 0 && attr->ctime.v1.useconds < 1000000)
		/* v1 timestamp */
        ufs_v1_to_v2(&attr->ctime.v1, &dinode->ctime.v2);
    else
        dinode->ctime.v2 = mtime64(NULL);

    if (attr->mtime.v1.seconds > 0 && attr->mtime.v1.useconds < 1000000)
		/* v1 timestamp */
        ufs_v1_to_v2(&attr->mtime.v1, &dinode->mtime.v2);
    else
        dinode->mtime.v2 = dinode->ctime.v2;

    if (attr->atime.v1.seconds > 0 && attr->atime.v1.useconds < 1000000)
		/* v1 timestamp */
        ufs_v1_to_v2(&attr->atime.v1, &dinode->atime.v2);
    else
        dinode->atime.v2 = dinode->ctime.v2;

    switch (dinode->mode & UFS_IFMT)	{
    default:
        ufs_panic("%s Can't allocate %o type.", __func__, dinode->mode);
        ufs_inode_deallocate(vdisk, *ino);
        return EINVAL;
    case UFS_IFCHR:
        /* FALLTHROUGH */
    case UFS_IFBLK:
        dinode->nlink     = 1;
#if 0
        minode.device           = attr->device;
        minode.dinode.addr[0]   = minode.device;
#endif
        break;
    case UFSBSD_IFFIFO:
        /* FALLTHROUGH */
    case UFSBSD_IFSOCK:
        /* FALLTHROUGH */
    case UFSBSD_IFLNK:
        /* FALLTHROUGH */
    case UFS_IFREG:
        dinode->nlink = 1;
        break;
    case UFS_IFDIR:
        dinode->nlink = 2;	/* . + .. */
        if ((error = ufs_datablock_expand(vdisk, minode, sizeof(*dir) * 2 ))) {
            ufs_inode_deallocate(vdisk, *ino);
            return error;
        }
		blk = dinode->addr[0];
		buf = ufs_disk_get_buf(vdisk->disk);
		if (!buf) {
            ufs_panic("%s out of memory", __func__);
            return ENOMEM;
        }
        if (ufs_vdisk_read(vdisk, buf, blk)) {
            ufs_disk_free_buf(vdisk->disk, buf);
            ufs_inode_deallocate(vdisk, *ino);
            return EIO;
        }

        dir = (UFSDIR *)buf;
        strcpy(dir[0].name, ".");
        dir[0].inode_number = *ino;
        strcpy(dir[1].name, "..");
        dir[1].inode_number = parent_dir->inode_number;
        if (ufs_vdisk_write(vdisk, buf, blk)) {
            ufs_disk_free_buf(vdisk->disk, buf);
            return EIO;
        }
        ufs_disk_free_buf(vdisk->disk, buf);
        break;
    }

    ufs_inode_writeback(minode);
#if 0
    wtof("%s created %s, inode=%u", __func__, filename, minode->inode_number);
    ufs_inode_dump(minode);
    wtof("%s parent dir inode=%d", __func__, parent_dir->inode_number);
    ufs_inode_dump(parent_dir);
#endif
    ufs_inode_rel(minode);

    /* Link this inode to parent directory. */
#if 1
	{
		/* make sure filename does not include the path portion */
		char *p = strrchr(filename, '/');
		if (p) strcpy(filename, p+1);
	}
#endif
    if ((error = ufs_directory_add_entry(vdisk, parent_dir, *ino, filename))) {
        ufs_panic("%s can't add dirent.", __func__);
        return error;
    }

    return 0;
}
