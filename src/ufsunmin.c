#include "ufs/sys.h"
#include "ufs/file.h"
#include "ufs/mount.h"

int ufs_unmount_inode(UFS *ufs, UFSMIN *minode)
{
    UFSSYS      *sys    		= ufs->sys;
    UFSVDISK    *vdisk  		= NULL;
    UFSDISK		*disk   		= NULL;
    INT32       rc 				= 0;
    int			lockrc			= -1;
    unsigned    count, n;

	if (!ufs || !minode) {
		wtof("%s: MISSING required parms", __func__);
		rc = EINVAL;
		goto quit;
	}

	if (!ufs_inode_isdir(&minode->dinode)) {
		wtof("%s: minode is not a directory", __func__);
		rc = ENOTDIR;
		goto quit;
	}

	if (!minode->mounted_vdisk) {
		wtof("%s: no disk is mounted on this minode", __func__);
		rc = EPERM;
		goto quit;
	}

	vdisk = minode->mounted_vdisk;
	if (vdisk->mounted_minode != minode) {
		wtof("%s vdisk mounted_minode does not match this minode", __func__);
		rc = EPERM;
		goto quit;
	}

	lockrc = lock(sys, LOCK_EXC);

	/* remove the minode from the system mountpoint array */
	count = array_count(&sys->mountpoint);
	for(n=count; n > 0; n--) {
		UFSMIN *mp = array_get(&sys->mountpoint, n);
		if (!mp) continue;
		if (mp == minode) {
			mp = array_del(&sys->mountpoint, n);
			// wtof("%s: minode=%08X removed from system mountpoints", __func__, mp);
			break;
		} 
	}

	/* remove the mount information from the minode and vdisk */
	minode->mounted_vdisk = NULL;
	vdisk->mounted_minode = NULL;
	// wtof("%s: minode=%08X unmounted vdisk=%08X", __func__, minode, vdisk);

	/* decrement the use count on this minode */
	ufs_inode_rel(minode);

quit:
	if (lockrc==0) unlock(sys, LOCK_EXC);
	return rc;
}
