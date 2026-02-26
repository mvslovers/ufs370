#include "ufs/sys.h"
#include "ufs/file.h"
#include "ufs/mount.h"

int ufs_unmount_vdisk(UFS *ufs, UFSVDISK *vdisk)
{
    UFSSYS      *sys    		= ufs->sys;
    UFSMIN		*minode  		= NULL;
    INT32       rc 				= 0;

	if (!ufs || !vdisk) {
		wtof("%s: MISSING required parms", __func__);
		rc = EINVAL;
		goto quit;
	}

	minode = vdisk->mounted_minode;
	if (!minode) {
		wtof("%s: no minode is mounted to this vdisk", __func__);
		rc = EPERM;
		goto quit;
	}

	/* we have the minode so let ufs_unmount_minode() do the heavy lifting */
	rc = ufs_unmount_inode(ufs, minode);

quit:
	return rc;
}
