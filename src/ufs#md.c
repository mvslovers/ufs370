#include "ufs/sys.h"
#include "ufs/file.h"

INT32   
ufs_mkdir(UFS *ufs, const char *path)
{
    UFSSYS      *sys    	= ufs->sys;
    UFSCWD      *cwd    	= ufs->cwd;
    ACEE        *acee   	= ufs->acee;
    UINT32      ino     	= 0;
    UFSMIN      *minode 	= NULL;
    UFSMIN		*dir_minode	= NULL;
    UFSMIN		*last_minode= NULL;
    UFSVDISK    *vdisk  	= NULL;
    INT32       rc = 0;
    UFSATTR     attr = {0};
    char        owner[9];
    char        group[9];
    char        fullpath[UFS_PATH_MAX+1]="";
    char		*p;
    char        temppath[UFS_PATH_MAX+1]="";

    if (!path) {
		rc = EINVAL;
		goto quit;
	}

	// wtof("%s: path=\"%s\"", __func__, path);

	if (path[0]=='/') {
		strncpy(temppath, path, UFS_PATH_MAX);
	}
	else if (cwd) {
		snprintf(temppath, UFS_PATH_MAX, "%s/%s", cwd->path, path);
	}
	
	if (temppath[0]==0) {
		strcpy(temppath, "/");
	}
	
	rc = ufs_lookup_minode(ufs, path, &dir_minode, &last_minode, fullpath);
	
	// wtof("%s: fullpath=\"%s\" rc=%d %s", __func__, fullpath, rc, rc ? strerror(rc) : "FILE EXIST");
	// wtof("%s: dir_minode=%08X, last_minode=%08X", __func__, dir_minode, last_minode);
	
    if (rc==0) {
        /* path name already exist */
        rc = EEXIST;
        goto quit;
    }
    if (rc != ENOENT) goto quit;

	/* lookup was ENOENT so we have to make sure fullpath matches the temppath */
	// wtof("%s: temppath=\"%s\"", __func__, temppath);
	if (strcmp(fullpath,temppath)!=0) {
		// wtof("%s: fullpath and temppath NOMATCH", __func__);
		// wtof("%s: fullpath=\"%s\"", __func__, fullpath);
		// wtof("%s: temppath=\"%s\"", __func__, temppath);
		rc = ENOENT;
		goto quit;
	}
		
	minode = last_minode;
	if (minode) {
		last_minode = NULL;
	}
	else {
		minode = dir_minode;
		dir_minode = NULL;
	}

	vdisk = minode->vdisk;

    /* check for mount point on this inode */
    if (minode->mounted_vdisk) {
        /* follow mount point to mounted disk */
        vdisk = minode->mounted_vdisk;
        /* release the current inode */
        ufs_inode_rel(minode);
        /* get the root inode on the mounted disk */
        minode = ufs_inode_get(vdisk, UFS_ROOT_INODE);
        if (!minode) {
			rc = ENOENT;
			goto quit;
		}
    }

    /* make sure we can write to file system */
    if (minode->vdisk->disk->readonly) {
		rc = EROFS;
		goto quit;
	}

    /* make sure we have write access to the directory */
    if (!ufs_inode_auth_write(acee, minode)) {
        rc = EACCES;
        goto quit;
    }


    /* allocate new directory */
    if (acee && memcmp(acee->aceeacee, "ACEE", 4)==0) {
        /* get owner and group from security environment */
        memcpyp(owner, sizeof(owner), &acee->aceeuser[1], acee->aceeuser[0], 0);
        memcpyp(group, sizeof(group), &acee->aceegrp[1], acee->aceegrp[0], 0);
    }
    else {
        /* plug in some *safe* defaults */
        strcpy(owner, "HERC01");
        strcpy(group, "ADMIN");
    }
    attr.owner  	= owner;
    attr.group  	= group;
    attr.mode   	= UFS_IFDIR | ufs->create_perm;
    attr.atime.v2 	= mtime64(NULL);
    attr.mtime  	= attr.atime;
    attr.ctime  	= attr.atime;

    /* allocate new directory */
    rc = ufs_file_allocate(minode, fullpath, &attr, &ino);
    /* wtof("%s rc=%d, ino=%u", __func__, rc, ino); */
    if (rc) goto quit;

    /* release the inode of the parent directory */
    ufs_inode_rel(minode);

    /* get minode for this inode number */
    minode = ufs_inode_get(vdisk, ino);
    if (!minode) {
		rc = ENOENT;
	}
    /* wtodumpf(minode, sizeof(UFSMIN), "%s Child=%u", __func__, ino); */

quit:
	if (minode)		ufs_inode_rel(minode);
	if (dir_minode) ufs_inode_rel(dir_minode);
	if (last_minode) ufs_inode_rel(last_minode);
	return rc;
}

#if 0	/* old code */
__asm__("\n&FUNC    SETC 'ufs_mkdir'");
INT32   ufs_mkdir(UFS *ufs, const char *path)
{
    UFSSYS      *sys    = ufs->sys;
    UFSCWD      *cwd    = ufs->cwd;
    ACEE        *acee   = ufs->acee;
    UINT32      ino     = 0;
    UFSMIN      *minode = NULL;
    UFSVDISK    *vdisk  = NULL;
    INT32       rc = 0;
    UFSATTR     attr = {0};
    char        owner[9];
    char        group[9];
    char        fullpath[UFS_PATH_MAX+1]="";
#if 1
    char		*p;
    char        temppath[UFS_PATH_MAX+1]="";
#endif

    if (!path) return 0;

#if 1
	// wtof("%s: in path=\"%s\"", __func__, path);

	strncpy(temppath, path, sizeof(temppath));
	if (p=strrchr(temppath, '/')) {
		*p = 0;
	}
	else if (cwd) {
		strcpy(temppath, cwd->path);
	}
	
	if (temppath[0]==0) {
		strcpy(temppath, "/");
	}
#endif

	// wtof("%s: ufs_lookup path=\"%s\"", __func__, path);
    rc = ufs_lookup(ufs, UFS_CREATE, path, &vdisk, &ino, fullpath);
    // wtof("%s: rc=%d, vdisk=%08X, inode=%u, fullpath=\"%s\"", __func__, rc, vdisk, ino, fullpath);
    if (!vdisk) {
        wtof("%s ufs_lookup did not return a UFSVDISK handle", __func__);
        rc = ENOENT;
        goto quit;
    }

    if (rc==0) {
        /* path name already exist */
        rc = EEXIST;
        goto quit;
    }
    if (rc!=ENOENT) goto quit;

#if 1
	/* lookup was ENOENT so we have to make sure fullpath matches the temppath */
	// wtof("%s: temppath=\"%s\"", __func__, temppath);
	if (strcmp(fullpath,temppath)!=0) {
		// wtof("%s: fullname NOMATCH", __func__);
		rc = ENOENT;
		goto quit;
	}
#endif

    /* make sure we can write to file system */
    if (vdisk->disk->readonly) return EROFS;

    minode = ufs_inode_get(vdisk, ino);
    if (!minode) {
        rc = ENOENT;
        goto quit;
    }

    /* allocate new directory */
    if (acee && memcmp(acee->aceeacee, "ACEE", 4)==0) {
        /* get owner and group from security environment */
        memcpyp(owner, sizeof(owner), &acee->aceeuser[1], acee->aceeuser[0], 0);
        memcpyp(group, sizeof(group), &acee->aceegrp[1], acee->aceegrp[0], 0);
    }
    else {
        /* plug in some *safe* defaults */
        strcpy(owner, "HERC01");
        strcpy(group, "ADMIN");
    }
    attr.owner  = owner;
    attr.group  = group;
    attr.mode   = UFS_IFDIR | ufs->create_perm;
#if 0
    attr.atime  = ufs_timeval(NULL);
#else
    attr.atime.v2 = mtime64(NULL);
#endif
    attr.mtime  = attr.atime;
    attr.ctime  = attr.atime;

    /* allocate new directory */
    rc = ufs_file_allocate(minode, path, &attr, &ino);
    /* wtof("%s rc=%d, ino=%u", __func__, rc, ino); */
    if (rc) goto quit;

    /* release the inode of the parent directory */
    ufs_inode_rel(minode);

    /* get minode for this inode number */
    minode = ufs_inode_get(vdisk, ino);
    if (!minode) return ENOENT;
    /* wtodumpf(minode, sizeof(UFSMIN), "%s Child=%u", __func__, ino); */

quit:
    /* release the inode */
    if (minode) ufs_inode_rel(minode);
    return rc;
}
#endif	/* old code */
