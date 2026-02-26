#include "ufs/sys.h"
#include "ufs/file.h"


__asm__("\n&FUNC    SETC 'ufs_remove'");
INT32 ufs_remove(UFS *ufs, const char *path)
{
    UFSSYS      *sys    	= ufs->sys;
    UFSCWD      *cwd    	= ufs->cwd;
    ACEE        *acee   	= ufs->acee;
    UINT32      ino     	= 0;
    UFSMIN      *dir_minode = NULL;
    UFSMIN      *last_minode= NULL;
    INT32       rc = 0;
    char        fullpath[UFS_PATH_MAX+1];
    char        *name;

    // wtof("%s enter path=\"%s\"", __func__, path);

    rc = ufs_lookup_minode(ufs, path, &dir_minode, &last_minode, fullpath);
    // wtof("%s ufs_lookup_minode() fullpath=\"%s\" rc=%d", __func__, fullpath, rc);
    if (rc) goto quit;

    if (!last_minode) {
        rc = ENOENT;
        goto quit;
    }

    if (ufs_inode_isdir(&last_minode->dinode)) {
        /* wtof("%s \"%s\" is a directory", __func__, path); */
        rc = EISDIR;
        goto quit;
    }

    /* locate parent directory inode */
    name = strrchr(fullpath, '/');
    if (!name) {
        rc = ENOENT;
        goto quit;
    }
    *name++ = 0;

    if (dir_minode->vdisk->disk->readonly) {
        rc = EROFS;
        goto quit;
    }

    if (last_minode->vdisk->disk->readonly) {
        rc = EROFS;
        goto quit;
    }

    /* make sure we have write access to the parent */
    if (!ufs_inode_auth_write(acee, dir_minode)) {
        /* wtof("%s user does not have WRITE access to \"%s\"", __func__, fullpath); */
        rc = EACCES;
        goto quit;
    }

    /* make sure we have write access to the child */
    if (!ufs_inode_auth_write(acee, last_minode)) {
        /* wtof("%s user does not have WRITE access to \"%s\"", __func__, name"); */
        rc = EPERM;
        goto quit;
    }

	ufs_inode_rel(last_minode);
	last_minode = NULL;

    /* deallocate directory */
    rc = ufs_file_deallocate(dir_minode, name);
    /* wtof("%s ufs_file_deallocate(%08X,\"%s\") rc=%d", __func__, dir_minode, name, rc); */

quit:
    if (dir_minode) ufs_inode_rel(dir_minode);
	if (last_minode) ufs_inode_rel(last_minode);
	// wtof("%s: exit rc=%d %s", __func__, rc, rc ? strerror(rc) : "SUCCESS");
	return rc;
}
