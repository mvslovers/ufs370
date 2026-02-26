#include "ufs/sys.h"
#include "ufs/file.h"

INT32   
ufs_chgdir(UFS *ufs, const char *path)
{
    UFSCWD      *cwd    	= ufs->cwd;
    UINT32      ino     	= 0;
    UFSVDISK    *vdisk  	= NULL;
    UFSMIN      *minode 	= NULL;
    UFSMIN		*dir_minode	= NULL;
    UFSMIN		*last_minode= NULL;
    char        fullpath[UFS_PATH_MAX+1] = "";
    INT32       rc;

	// wtof("%s: path=\"%s\"", __func__, path);

	rc = ufs_lookup_minode(ufs, path, &dir_minode, &last_minode, fullpath);
	// wtof("%s: fullpath=\"%s\" rc=%d %s", __func__, fullpath, rc, rc ? strerror(rc) : "SUCCESS");
	if (rc) goto quit;

	minode = last_minode;
	if (minode) last_minode = NULL;
	if (!minode) {
		minode = dir_minode;
		dir_minode = NULL;
	}

    /* make sure this inode is a directory inode */
    if (!ufs_inode_isdir(&minode->dinode)) {
        rc = ENOTDIR;
        goto quit;
    }

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

    /* success, release the previous current directory inode */
    if (cwd->cur_dir) ufs_inode_rel(cwd->cur_dir);

    /* save the new current directory inode */
    cwd->cur_dir = minode;
    minode = NULL;

    /* update the path name of the current working directory */
    strcpyp(cwd->path, sizeof(cwd->path), fullpath, 0);

quit:
	if (minode) ufs_inode_rel(minode);
	if (dir_minode) ufs_inode_rel(dir_minode);
	if (last_minode) ufs_inode_rel(last_minode);
	return rc;
}
