#include "ufs/sys.h"
#include "ufs/file.h"
#include "ufs/inode.h"

static INT32 directory_list_subr(UFSVDISK *vdisk, void *ctx, UINT32 blk, UINT32 sz);
static int directory_name_sort(const void *v1, const void *v2);

UFSDDESC *ufs_diropen(UFS *ufs, const char *path, const char *pattern)
{
    UFSCWD      *oldcwd 		= ufs->cwd;
    UFSDDESC    *ddesc  		= NULL;
	UFSMIN		*dir_minode		= NULL;
	UFSMIN		*last_minode	= NULL;
	UFSMIN		*minode			= NULL;
	UFSVDISK	*vdisk			= NULL;
    INT32       rc;
    UINT32      count;
    char        fullpath[UFS_PATH_MAX+1] = "";

	// wtof("%s: enter path=\"%s\"", __func__, path);

    if (!path || !*path) path = oldcwd->path;

	rc = ufs_lookup_minode(ufs, path, &dir_minode, &last_minode, fullpath);
	if (rc) goto quit;

	minode = last_minode;
	if (!minode) {
		/* likely path was simple name like "/" */
		minode = dir_minode;
	}
	
	if (!minode) {
		rc = ENOENT;
		goto quit;
	}

	vdisk = minode->vdisk;

	if (minode->mounted_vdisk) {
		// wtof("%s:   mount on dir minode=%08X vdisk=%08X ino=%u", 
		// 	__func__, minode, minode->vdisk, minode->inode_number);
		/* switch to mounted vdisk */
		vdisk = minode->mounted_vdisk;
		/* release this minode */
		ufs_inode_rel(minode);
		/* get the minode for the vdisk root inode */
		minode = ufs_inode_get(vdisk, UFS_ROOT_INODE);
		// wtof("%s:   vdisk=%08X, minode=%08X", __func__, vdisk, minode);
		if (!minode) {
			rc = ENOENT;
			goto quit;
		}
		/* bump the use count for this minode */
		ufs_inode_use(minode);
	}

	// wtodumpf(minode, sizeof(UFSMIN), "%s: minode", __func__);
	
    /* the user must have read access to the directory inode */
    if (!ufs_inode_auth_read(ufs->acee, minode)) {
        /* wtof("%s user does not have %s access to directory", __func__, "READ"); */
        errno = EPERM;
        goto quit;
    }

    /* Success, allocate a UFSDDESC handle */
    ddesc = calloc(1, sizeof(UFSDDESC));
    if (!ddesc) {
        errno = ENOMEM;
        goto quit;
    }

    /* Copy the inode and path info to the UFSDDESC handle */
    ddesc->minode = minode;
    strcpyp(ddesc->path, sizeof(ddesc->path), fullpath, 0);
    ddesc->arg1 = ufs;
    ddesc->arg2 = (void*) pattern;

    /* create directory list array */
    rc = ufs_datablock_foreach(vdisk, minode, directory_list_subr, ddesc);

    count = array_count(&ddesc->dlist);

    // wtof("%s:   count = %u", __func__, count);

    if (count > 1) {
        /* wtof("%s sorting %u directory records", __func__, count); */
        qsort(ddesc->dlist, count, sizeof(UFSDLIST *), directory_name_sort);
    }

quit:
    if (last_minode) ufs_inode_rel(last_minode);
    if (dir_minode) ufs_inode_rel(dir_minode);
    
    // wtof("%s: exit desc=%08X rc=%d %s", __func__, ddesc, rc, rc ? strerror(rc) : "SUCCESS");
    return ddesc;
}

__asm__("\n&FUNC    SETC 'directory_list_subr'");
static INT32
directory_list_subr(UFSVDISK *vdisk, void *ctx, UINT32 blk, UINT32 sz)
{
    INT32   	ret 		= 0;
    UFSDDESC 	*p 			= (UFSDDESC *)ctx;
    UFS			*ufs		= p->arg1;
    const char	*pattern	= p->arg2;
    UFSDLIST 	*l;
    UFSDIR  	*dir;
    void    	*buf;
    UINT32  	i, n;
    UFSDIN  	*dinode;
    UFSMIN  	*minode;
    UINT32  	maxinode 	= ufs_max_inode(vdisk->disk->blksize, vdisk->disk->sb.datablock_start_sector);

	// wtof("%s:   enter vdisk=%08X, ctx=%08X, blk=%u, sz=%u, maxinode=%u", 
	// 	__func__, vdisk, ctx, blk, sz, maxinode);

    /* ufs_panic("%s match start blk=%u", __func__, blk); */
    buf = ufs_disk_get_buf(vdisk->disk);
    if (!buf) {
		wtof("%s:   ufs_disk_get_buf() failed", __func__);
        ufs_panic("%s out of memory", __func__);
        ret = ENOMEM;
        goto quit;
    }

    if (ufs_vdisk_read(vdisk, buf, blk)) {
		wtof("%s:   ufs_vdisk_read() read error block %u", __func__, blk);
        ufs_panic("%s read error block %u", __func__, blk);
        ret = EIO;
        goto quit;
    }

    dir = (UFSDIR *)buf;
    n = sz / sizeof(*dir);

#if 0
    wtof("%s:   p=%08X, blk=%u, sz=%u, n=%u", __func__, p, blk, sz, n);
    wtodumpf(p, sizeof(UFSDDESC), "%s UFSDDESC", __func__);
    wtodumpf(dir, sizeof(UFSDISK) * n, "%s DIR buffer", __func__);
#endif

    for (i = 0; i < n; i++, dir++) {
		int	has_mounted_disk = 0;
		int	is_directory = 0;
		char *ddname = NULL;
        UINT32 len = strlen(dir->name);

        if (len > UFS_NAME_MAX || strchr(dir->name, ' ') || dir->inode_number > maxinode) {
            wtodumpf(dir, sizeof(UFSDIR), "%s Invalid DIR entry", __func__);
            continue;
        }

        /* get inode for this directory entry */
        minode = ufs_inode_get(vdisk, dir->inode_number);
        if (!minode) continue;

		/* if the caller specified a pattern and 
		 * this dir name doesn't match then skip it. 
		 */
		if (pattern && *pattern && !__patmat(dir->name, pattern)) {
			ufs_inode_rel(minode);
			continue;
		}

        l = calloc(1, sizeof(UFSDLIST));
        if (!l) {
            ufs_inode_rel(minode);
            ret = ENOMEM;
            goto quit;
        }

        dinode = &minode->dinode;

		/* if this is a directory */
        if (ufs_inode_isdir(dinode)) {
			is_directory = 1;
			/* if this directory has a mounted disk on it */
			if (minode->mounted_vdisk) {
				/* has_mounted_disk is true */
				UFSDISK *disk = minode->mounted_vdisk->disk;
				
				has_mounted_disk = 1;
				ddname = disk->ddname;
			}
			
			if (p->minode->inode_number == UFS_ROOT_INODE && dir->inode_number == UFS_ROOT_INODE) {
				/* all inode_numbers that are UFS_ROOT_INODE are always mounted */
				UFSDISK		*disk;
				UFSMIN 		*pminode;

				has_mounted_disk = 1;

				if (strcmp(dir->name, ".")==0) {
					// wtof("%s: SELF '.'", __func__);
					disk = vdisk->disk;
					ddname = disk->ddname;
				}
				else if (strcmp(dir->name, "..")==0) {
					// wtof("%s: PARENT '..'", __func__);

					pminode = ufs_inode_mounted_vdisk(ufs, vdisk);
					if (pminode) {
						// wtof("%s: pminode->vdisk=%08X", __func__, pminode->vdisk);
						// wtof("%s: pminode->mounted_vdisk=%08X", __func__, pminode->mounted_vdisk);
						disk = pminode->vdisk->disk;
						// wtof("%s: mounted 2 disk=%08X", __func__, disk);
						ufs_inode_rel(pminode);
					}
					else {
						disk = vdisk->disk;
						// wtof("%s: mounted 3 disk=%08X", __func__, disk);
					}
					ddname = disk->ddname;
				}
				// wtof("%s: mounted ddname=\"%s\"", __func__, ddname);
			}
		}

#if 0
        wtof("%s i=%d, dir=%08X, calling array_add() l=%08X, p->dlist=%08X",
             __func__, i, dir, l, p->dlist);
        wtodumpf(p, sizeof(UFSDDESC), "%s UFSDDESC before", __func__);
#endif
        array_add(&p->dlist, l);
#if 0
        wtodumpf(p, sizeof(UFSDDESC), "%s UFSDDESC after", __func__);
#endif
        l->inode_number = dir->inode_number;
        strcpy(l->name, dir->name);

        if (has_mounted_disk) {
			int namelen = strlen(l->name);
			if (namelen < sizeof(l->name)-16) {
				sprintf(&l->name[namelen], " -> DD:%s", ddname);
			}
		}

        l->filesize     = dinode->filesize;

		/* convert v1 timestamps to v2 timestamps */
		if (dinode->ctime.v1.useconds < 1000000) {
			/* convert seconds and useconds to utime64_t value */
			ufs_v1_to_v2(&dinode->ctime.v1, &l->ctime);
		}
		else {
	        l->ctime    = dinode->ctime.v2;
		}

		if (dinode->mtime.v1.useconds < 1000000) {
			/* convert seconds and useconds to utime64_t value */
			ufs_v1_to_v2(&dinode->mtime.v1, &l->mtime);
		}
		else {
	        l->mtime    = dinode->mtime.v2;
		}

		if (dinode->atime.v1.useconds < 1000000) {
			/* convert seconds and useconds to utime64_t value */
			ufs_v1_to_v2(&dinode->atime.v1, &l->atime);
		}
		else {
			l->atime    = dinode->atime.v2;
		}

        l->nlink        = dinode->nlink;
        l->codepage     = dinode->codepage;
        strcpy(l->owner, dinode->owner);
        strcpy(l->group, dinode->group);
        /* we're going to use 'm' to indicate a directory with a mounted disk */
        l->attr[0]      = has_mounted_disk ? 'm' : is_directory ? 'd' : '-';
        l->attr[1]      = dinode->mode & 0400 ? 'r' : '-';
        l->attr[2]      = dinode->mode & 0200 ? 'w' : '-';
        l->attr[3]      = dinode->mode & 0100 ? 'x' : '-';
        l->attr[4]      = dinode->mode & 040 ? 'r' : '-';
        l->attr[5]      = dinode->mode & 020 ? 'w' : '-';
        l->attr[6]      = dinode->mode & 010 ? 'x' : '-';
        l->attr[7]      = dinode->mode & 04 ? 'r' : '-';
        l->attr[8]      = dinode->mode & 02 ? 'w' : '-';
        l->attr[9]      = dinode->mode & 01 ? 'x' : '-';
        ufs_inode_rel(minode);
    }

quit:
	if (buf) ufs_disk_free_buf(vdisk->disk, buf);

	if (ret) errno = ret;

	// wtof("%s:   exit rc=%d %s", __func__, ret, ret ? strerror(ret) : "SUCCESS");

    return ret;
}

__asm__("\n&FUNC    SETC 'directory_name_sort'");
static int directory_name_sort(const void *v1, const void *v2)
{
    const UFSDLIST  *p1 = *(UFSDLIST**) v1;
    const UFSDLIST  *p2 = *(UFSDLIST**) v2;

    /* wtof("%s(\"%s\",\"%s\")", __func__, p1->name, p2->name); */
    return strcmp(p1->name, p2->name);
}
