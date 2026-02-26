#include "ufs/sys.h"
#include "ufs/file.h"

INT32   
ufs_rmdir(UFS *ufs, const char *path)
{
    ACEE        *acee   	= ufs->acee;
    UFSMIN      *dir_minode = NULL;
    UFSMIN      *last_minode= NULL;
    INT32       rc = 0;
    char        fullpath[UFS_PATH_MAX+1];
    char        *name;

    // wtof("%s enter", __func__);
	rc = ufs_lookup_minode(ufs, path, &dir_minode, &last_minode, fullpath);
	if (rc) goto quit;

	/* dir_minode is the parent directory */
	/* last_minode is the directory we want to delete */

	if (!dir_minode) {
		wtof("%s ufs_lookup_minode() didn't return a %s", __func__, "dir_minode");
        rc = ENOENT;
        goto quit;
	}
	
	if (!last_minode) {
		wtof("%s ufs_lookup_minode() didn't return a %s", __func__, "last_minode");
        rc = ENOENT;
        goto quit;
	}

    name = strrchr(fullpath, '/');
    if (!name) {
		wtof("%s strrchr() didn't find '/'", __func__);
        rc = ENOENT;
        goto quit;
    }
    name++;

    if (!ufs_inode_isdir(&last_minode->dinode)) {
        wtof("%s \"%s\" is not a directory", __func__, name);
        rc = ENOTDIR;
        goto quit;
    }

    if (dir_minode->vdisk->disk->readonly) {
		wtof("%s parent disk is read only", __func__);
        rc = EROFS;
        goto quit;
    }

    if (last_minode->vdisk->disk->readonly) {
		wtof("%s child disk is read only", __func__);
        rc = EROFS;
        goto quit;
    }

    /* make sure we have write access to the parent */
    if (!ufs_inode_auth_write(acee, dir_minode)) {
        wtof("%s user does not have WRITE access to \"%s\" parent directory", __func__, name);
        rc = EACCES;
        goto quit;
    }

    /* make sure we have write access to the child */
    if (!ufs_inode_auth_write(acee, last_minode)) {
        wtof("%s user does not have WRITE access to \"%s\" directory", __func__, name);
        rc = EACCES;
        goto quit;
    }

	/* make sure this directory is not a mounted disk */
	if (last_minode->mounted_vdisk) {
		/* this directory has a mounted disk */
		UFSDISK *disk = last_minode->mounted_vdisk->disk;
		wtof("%s DD:%s is mounted on this directory", __func__, disk->ddname);
		rc = EPERM;
		goto quit;
	}

    /* deallocate directory */
    rc = ufs_file_deallocate(dir_minode, name);
    // wtof("%s ufs_file_deallocate() rc=%d", __func__, rc);

quit:
	if (dir_minode) ufs_inode_rel(dir_minode);
	if (last_minode) ufs_inode_rel(last_minode);
	return rc;
}
