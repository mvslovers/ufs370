#include "ufs/sys.h"
#include "ufs/file.h"
#include "ufs/mount.h"

#if 1 /* new code */
INT32 ufs_mount(UFS *ufs, const char *ddname, const char *path)
{
    UFSSYS      *sys    		= ufs->sys;
    UFSCWD      *cwd    		= ufs->cwd;
    ACEE        *acee   		= ufs->acee;
    UFSMIN		*dir_minode		= NULL;
    UFSMIN		*last_minode	= NULL;
    UINT32      ino     		= 0;
    UFSMIN      *minode 		= NULL;
    UFSVDISK    *vdisk  		= NULL;
    UFSVDISK	*ddvdisk		= NULL;
    UFSDISK		*disk   		= NULL;
    INT32       rc 				= 0;
    int			lockrc			= -1;
    unsigned    count, n;
    char        fullpath[UFS_PATH_MAX+1]="";

	if (!ufs || !ddname || !path) {
		wtof("%s: MISSING required parms", __func__);
		rc = EINVAL;
		goto quit;
	}

	lockrc = lock(sys, LOCK_EXC);

	// wtof("%s: ddname=\"%s\" path=\"%s\" ", __func__, ddname, path);

	/* find the vdisk for the ddname */
	count = array_count(&sys->vdisks);
	// wtof("%s: searching sys->vdisk array %u for ddname=\"%s\"", __func__, count, ddname);
	for(n=0; n < count; n++) {
		vdisk = sys->vdisks[n];
		if (!vdisk) continue;
		disk = vdisk->disk;
		if (!disk) continue;
		
		if (strcmp(disk->ddname, ddname)==0) {
			ddvdisk = vdisk;
			break;
		}
	}
	if (!ddvdisk) {
		// wtof("%s: search failed for ddname=\"%s\"", __func__, ddname);
		rc = ENOENT;
		goto quit;
	}
	// wtof("%s: SUCCESS \"%s\" vdisk=%08X", __func__, ddname, ddvdisk);
	// wtodumpf(ddvdisk, sizeof(UFSVDISK), "%s vdisk", ddname);

	/* check the disk we're mounting to make sure it has a valid root
	 * directory minode
	 */
	minode = ufs_inode_get(ddvdisk, UFS_ROOT_INODE);
	if (!minode) {
		// wtof("%s: disk on %s to be mounted does not have a root inode", __func__, ddname);
		rc = EIO;
		goto quit;
	}

	if (!ufs_inode_isdir(&minode->dinode)) {
		// wtof("%s: disk on %s root inode is not a directory", __func__, ddname);
		rc = ENOTDIR;
		goto quit;
	}
	if (minode->dinode.nlink < 2) {
		// wtof("%s: disk on %s root inode has nlink < 2", __func__, ddname);
		rc = EINVAL;
		goto quit;
	}
	ufs_inode_rel(minode);
	minode = NULL;

	/* find the last minode for the path name --> last_minode*/
	rc = ufs_lookup_minode(ufs, path, &dir_minode, &last_minode, NULL);
	if (rc) {
		// wtof("%s: ufs_lookup_minode() failed. rc=%d", __func__, rc);
		goto quit;
	}

	if (!last_minode) {
		// wtof("%s: ufs_lookup_minode() did not return last_minode", __func__);
		rc = ENOENT;
		goto quit;
	}

	/* make sure this vdisk is not mounted already */
	if (ddvdisk->mounted_minode) {
		wtof("%s: DD:%s is already mounted", __func__, ddname);
		goto quit;
	}

	count = array_count(&sys->mountpoint);
	for(n=0; n < count; n++) {
		if (sys->mountpoint[n] == last_minode) {
			wtof("%s: a disk is alreaded mounted on \"%s\"", __func__, path);
			goto quit;
		}
	}

    ufs_inode_use(last_minode);  /* bump the use count */
	minode = last_minode;

	// wtof("%s:- - - - - - - - - - - - - - - - - - - - - - - - - - - - -", __func__);
	// wtodumpf(last_minode, sizeof(UFSMIN), "last_minode");

	if (!ufs_inode_isdir(&minode->dinode)) {
		// wtof("%s: path=\"%s\" is not a directory", __func__, path);
		rc = ENOTDIR;
		goto quit;
	}

    if (!ufs_inode_auth_write(acee, minode)) {
		// wtof("%s: user does not have WRITE access to \"%s\"", __func__, path);
		rc = EPERM;
		goto quit;
	}

	if (minode->mounted_vdisk) {
		// wtof("%s: a mount already exist for \"%s\"", __func__, path);
		rc = EBUSY;
		goto quit;
	}

	if (ddvdisk->mounted_minode) {
		// wtof("%s: ddname=\"%s\" is already mounted on another path", __func__, ddname);
		rc = EBUSY;
		goto quit;
	}

	/* bump the use count one more time on this directory */
    ufs_inode_use(minode);
    minode->type |= MINODE_TYPE_MOUNTPOINT;

    /* mount the vdisk to the directory */
	minode->mounted_vdisk = ddvdisk;

	/* the vdisk on the mounted disk points back to the directory minode
	 * that it is mounted on.
	 */
	ddvdisk->mounted_minode = minode;

	/* add this mount point to the sys->mountpoint array */
	array_add(&sys->mountpoint, minode);

	// wtodumpf(minode, sizeof(UFSMIN), "minode with vdisk=%08X on mounted_minode (at offset +0000C)", 
	// 	__func__, ddvdisk);
	// wtof("%s:- - - - - - - - - - - - - - - - - - - - - - - - - - - - -", __func__);

quit:
	if (lockrc==0) unlock(sys, LOCK_EXC);
    if (minode) ufs_inode_rel(minode);
	// wtof("%s: return rc=%d %s", __func__, rc, rc ? strerror(rc) : "SUCCESS");
	return rc;
}
#endif /* new code */
