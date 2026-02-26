#include "ufs/sys.h"
#include "ufs/file.h"
#include "ufs/mount.h"

static int ufs_fstab_mount(UFS *ufs, char *buf);

int ufs_fstab(UFS *ufs, const char *fn) 
{
	int			rc			= 0;
	UFSFILE		*fp			= ufs_fopen(ufs, fn, "r");
	char		*p			= NULL;
	char		buf[256];
	
	if (!fp) {
		rc = ENOENT;
		goto quit;
	}
	
	while (p=ufs_fgets(buf, sizeof(buf), fp)) {
		// wtof("%s: \"%s\"", __func__, buf);

		/* process this fstab line */
		rc = ufs_fstab_mount(ufs, buf);
	}
	
quit:
	if (fp) ufs_fclose(&fp);
	
	return rc;
}
	
static int ufs_fstab_mount(UFS *ufs, char *buf)
{
	UFSSYS		*sys		= ufs->sys;
	ACEE		*acee		= ufs->acee;
	UFSDISK		*disk		= NULL;
	UFSVDISK	*vdisk		= NULL;
	UFSMIN		*minode		= NULL;
	UFSMIN		*dir_minode	= NULL;
	UFSMIN		*last_minode= NULL;
	int			rc			= 0;
	int			lockrc		= 0;
	char		*p			= NULL;
	char		*ddname		= NULL;
	char		*dsname		= NULL;
	char		*path		= NULL;
	char		*fstype		= NULL;
	int			len;
	unsigned	n;
	unsigned	count;
	
	lockrc = lock(sys, LOCK_EXC);

	/* remove trailing '/n' and white space characters */
	for(len = strlen(buf) - 1; len >= 0; len--) {
		if (isspace(buf[len])) {
			buf[len] = 0;
			continue;
		}
		break;
	}
		
	/* get first paramter or comment */
	p = strtok(buf, " ,\t");
	if (!p) goto quit;
	// wtof("%s: #1=\"%s\"", __func__, p);
		
	if (p[0]=='#') goto quit;
		
	if (strncmp(p, "ddname=", 7)==0) {
		p+=7;
		while(*p==' ') p++;
		ddname = p;
	}
	else if (strncmp(p, "dsname=", 7)==0) {
		p+=7;
		while(*p==' ') p++;
		dsname = p;
	}
	else {
		len = strlen(p);
		if (len <= 8 && !strchr(p, '.')) {
			ddname = p;
		}
		else if (len <= 44 && strchr(p, '.') && !strchr(p, '(') && !strchr(p, ')')) {
			dsname = p;
		}
		else {
			wtof("%s: missing 'ddname=...' or 'dsname=...'", __func__);
			goto quit;
		}
	}
	// wtodumpf(ddname, strlen(ddname), "ddname");

	/* get second parameter (mount path name) */
	path = strtok(NULL, " ,\t");
	if (!path) {
		wtof("%s: missing path name for mount", __func__);
		goto quit;
	}
	// wtof("%s: #2=\"%s\"", __func__, path);

	/* make sure we're not trying to mount on top of the root path */
	if (strcmp(path, "/")==0) {
		wtof("%s: mount path is the root path '/' and is ignored", __func__);
		goto quit;
	}
		
	/* get third parameter (file system type) */
	fstype = strtok(NULL, " ,\t");
	if (!fstype) fstype="ufs";
	// wtof("%s: #3=\"%s\"", __func__, fstype);
		
	/* validate parms */
	count = array_count(&sys->vdisks);
	for(n=2; n <= count; n++) {
		UFSDISK *d;
			
		vdisk = array_get(&sys->vdisks, n);
		if (!vdisk) continue;

		d = vdisk->disk;
		if (!d) continue;

		if (ddname) {
			if (strcmp(d->ddname, ddname)==0) {
				disk = d;
				break;
			}
		}
		else if (dsname) {
			if (strcmp(d->dsname, dsname)==0) {
				disk = d;
				break;
			}
		}
	}

	if (!disk) {
		wtof("%s: disk not found for \"%s\"", __func__, ddname ? ddname : dsname);
			goto quit;
	}

	/* we may need these for messages in the code below */
	ddname = disk->ddname;
	dsname = disk->dsname;

	/* check the disk we're mounting to make sure it has a valid root
	* directory minode
	*/
	minode = ufs_inode_get(vdisk, UFS_ROOT_INODE);
	if (!minode) {
		wtof("%s: disk on %s to be mounted does not have a root inode", __func__, ddname);
		goto quit;
	}

	if (!ufs_inode_isdir(&minode->dinode)) {
		wtof("%s: disk on %s root inode is not a directory", __func__, ddname);
		goto quit;
	}

	if (minode->dinode.nlink < 2) {
		wtof("%s: disk on %s root inode has nlink < 2", __func__, ddname);
		goto quit;
	}
	ufs_inode_rel(minode);
	minode = NULL;

	/* get the minode for the mount path */
	rc = ufs_lookup_minode(ufs, path, &dir_minode, &last_minode, NULL);
	if (rc) {
		wtof("%s: path \"%s\" not found. rc=%d %s", __func__, rc, rc ? strerror(rc) : "SUCCESS");
		goto quit;
	}
		
	/* perform various checks on the mount path */
	if (!last_minode) {
		wtof("%s: mount path \"%s\" was not found", __func__, path);
		goto quit;
	}

	/* make sure this vdisk is not mounted already */
	if (vdisk->mounted_minode) {
		wtof("%s: DD:%s is already mounted", __func__, ddname);
		goto quit;
	}

	count = array_count(&sys->mountpoint);
	for(n=0; n < count; n++) {
		if (sys->mountpoint[n] == last_minode) {
			wtof("%s: another disk is alreaded mounted on \"%s\"", __func__, path);
			goto quit;
		}
	}
		
	if (last_minode->mounted_vdisk) {
		wtof("%s: the mount path \"%s\" already has a disk mounted on it.", __func__, path);
		goto quit;
	}
		
	if (!ufs_inode_isdir(&last_minode->dinode)) {
		wtof("%s: path=\"%s\" is not a directory", __func__, path);
		goto quit;
	}

	if (!ufs_inode_auth_write(acee, last_minode)) {
		wtof("%s: user does not have WRITE access to \"%s\"", __func__, path);
		goto quit;
	}

	if (last_minode->mounted_vdisk) {
		wtof("%s: a mount already exist for \"%s\"", __func__, path);
		goto quit;
	}

	if (vdisk->mounted_minode) {
		wtof("%s: ddname=\"%s\" is already mounted on another path", __func__, ddname);
		goto quit;
	}

	/* bump the use count one more time on this directory */
	ufs_inode_use(last_minode);
    last_minode->type |= MINODE_TYPE_MOUNTPOINT;

	/* mount the vdisk to the directory */
	last_minode->mounted_vdisk = vdisk;

	/* the vdisk on the mounted disk points back to the directory 
	 * minode that it is mounted on.
	*/
	vdisk->mounted_minode = last_minode;

	/* add this mount point to the sys->mountpoint array */
	array_add(&sys->mountpoint, last_minode);

quit:
	if (lockrc==0) unlock(sys, LOCK_EXC);
	if (last_minode) ufs_inode_rel(last_minode);
	if (dir_minode) ufs_inode_rel(dir_minode);
	if (minode) ufs_inode_rel(minode);
	
	return rc;
}

