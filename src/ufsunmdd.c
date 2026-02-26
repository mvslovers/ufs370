#include "ufs/sys.h"
#include "ufs/file.h"
#include "ufs/mount.h"

int ufs_unmount_ddname(UFS *ufs, const char *ddname)
{
    UFSSYS      *sys    		= ufs->sys;
    UFSMIN		*minode  		= NULL;
    int		    rc 				= 0;
    int			lockrc			= -1;
    unsigned	n;
    unsigned 	count;
    char		upname[12];
    
	if (!ufs || !ddname) {
		wtof("%s: MISSING required parms", __func__);
		rc = EINVAL;
		goto quit;
	}

    for(n=0; ddname[n] && n < sizeof(upname)-1; n++) {
		upname[n] = toupper(ddname[n]);
	}
	upname[n] = 0;
	ddname = upname;

	lockrc = lock(sys, LOCK_EXC);

	/* find the vdisk for the ddname */
	count = array_count(&sys->vdisks);
	// wtof("%s: searching sys->vdisk array %u for ddname=\"%s\"", __func__, count, ddname);
	for(n=0; n < count; n++) {
		UFSVDISK	*vdisk 	= sys->vdisks[n];

		if (!vdisk) continue;
		
		if (strcmp(vdisk->disk->ddname, ddname)==0) {
			minode = vdisk->mounted_minode;
			break;
		}
	}

	if (!minode) {
		wtof("%s: search failed for ddname=\"%s\"", __func__, ddname);
		rc = ENOENT;
		goto quit;
	}

	/* we have the minode so let ufs_unmount_minode() do the heavy lifting */
	rc = ufs_unmount_inode(ufs, minode);

quit:
	if (lockrc==0) unlock(sys, LOCK_EXC);
	return rc;
}
