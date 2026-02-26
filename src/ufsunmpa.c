#include "ufs/sys.h"
#include "ufs/file.h"
#include "ufs/mount.h"

int ufs_unmount_path(UFS *ufs, const char *path)
{
    UFSSYS      *sys    		= ufs->sys;
    UFSMIN		*minode  		= NULL;
    INT32       rc 				= 0;

	if (!ufs || !path) {
		wtof("%s: MISSING required parms", __func__);
		rc = EINVAL;
		goto quit;
	}

	if (strcmp(path, "/")==0) {
		minode = sys->fsroot;
	}
	else {
		/* find the last minode for the path name --> last_minode*/
		rc = ufs_lookup_minode(ufs, path, NULL, &minode, NULL);
		if (rc) {
			wtof("%s: ufs_lookup_minode() failed. rc=%d", __func__, rc);
			goto quit;
		}

		if (!minode) {
			wtof("%s: ufs_lookup_minode() did not return minode", __func__);
			rc = ENOENT;
			goto quit;
		}
	}

	/* we have the minode so let ufs_unmount_minode() do the heavy lifting */
	rc = ufs_unmount_inode(ufs, minode);

quit:
	return rc;
}
