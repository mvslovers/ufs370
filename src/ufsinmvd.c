#include "ufs/sys.h"
#include "ufs/inode.h"

UFSMIN *
ufs_inode_mounted_vdisk(UFS *ufs, UFSVDISK *vdisk)
{
	UFSSYS		*sys		= ufs->sys;
	UFSMIN		*minode		= NULL;
	unsigned	n;
	unsigned	count;
	int			lockrc;
	
	lockrc = lock(sys, LOCK_SHR);
	
	count = array_count(&sys->mountpoint);
	for (n=0; n < count; n++) {
		UFSMIN	*m = sys->mountpoint[n];
		
		if (!m) continue;
		if (m->mounted_vdisk == vdisk) {
			minode = m;
			ufs_inode_use(minode);
			break;
		}
	}

quit:
	if (lockrc==0) unlock(sys, LOCK_SHR);
	return minode;
}
