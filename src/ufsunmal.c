#include "ufs/sys.h"
#include "ufs/file.h"
#include "ufs/mount.h"

int ufs_unmount_all(void)
{
    UFSSYS      *sys    		= ufs_sys_get();    /* allocated by __wsaget()  */
    UFS         *ufs    		= NULL;
    UFSVDISK    *vdisk  		= NULL;
    UFSDISK		*disk   		= NULL;
    INT32       rc 				= 0;
    int			lockrc			= -1;
    unsigned    count, n;

	if (!sys) {
		wtof("%s: file system not initialized", __func__);
		rc = EINVAL;
		goto quit;
	}

	ufs = ufsnew();
	if (!ufs) {
		wtof("%s: Unable to allocate UFS handle", __func__);
		rc = ENOMEM;
		goto quit;
	}

	lockrc = lock(sys, LOCK_EXC);

	/* remove the minode from the system mountpoint array */
	count = array_count(&sys->mountpoint);
	for(n=count; n > 0; n--) {
		UFSMIN *mp = array_get(&sys->mountpoint, n);
		if (!mp) continue;
		if (!mp->mounted_vdisk) continue;

		// vdisk = mp->mounted_vdisk;
		// disk  = vdisk->disk;
		// wtof("%s: unmounting DD:%s DSN:%s", __func__, disk->ddname, disk->dsname);

		rc = ufs_unmount_inode(ufs, mp);
	}

quit:
	if (lockrc==0) unlock(sys, LOCK_EXC);
	if (ufs) ufsfree(&ufs);
	return rc;
}
