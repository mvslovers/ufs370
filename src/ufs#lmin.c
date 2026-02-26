#include "ufs/sys.h"
#include "ufs/file.h"

/* ufs_lookup_minode() - get minodes for the directory and the last name of path */
/* returns error code or 0 upon success */

/* Note: The returned dir minode will always be a directory minode.
 * and the returned last minode will be either a dir or file minode.
 * 
 * The **dir and **last can be NULL in which case those minode's are
 * not returned to the caller.
 */
int ufs_lookup_minode(UFS *ufs, const char *path, UFSMIN **dir, UFSMIN **last, char *fullpath)
{
	int			rc		= 0;
    UFSSYS      *sys    = ufs->sys;
    UFSCWD      *cwd    = ufs->cwd;
    ACEE        *acee   = ufs->acee;
    UFSMIN		*minode = NULL;
    UFSMIN		*dir_minode = NULL;
    UFSVDISK	*vdisk  = NULL;
    UINT32		ino		= 0;
    char        temppath[UFS_PATH_MAX+1] = {0};
    char        *name;
    char		*next;
    int			len;
    int			i;

	// wtof("%s:- - - - - - - - - - - - - - - - - - - - - - - - - - - - -", __func__);
	// wtof("%s: enter path=\"%s\"", __func__, path);
	
	strncpy(temppath, path, UFS_PATH_MAX);
	temppath[UFS_PATH_MAX] = 0;
	
	if (temppath[0]=='/') {
		/* temppath has a full path name */
	}
	else if (cwd) {
		/* the supplied path is not a full path name
		 * so we need to append it to the cwd
		 * to build a full temppath name.
		 */
		strncpy(temppath, cwd->path, UFS_PATH_MAX);
		len = strlen(temppath);

		if (temppath[len-1] != '/') {
			temppath[len++] = '/';
		}

		for(i=0; path[i] && len < UFS_PATH_MAX; len++, i++) {
			temppath[len] = path[i];
		}

		temppath[len] = 0;
	}
	
	/* make sure we have a full temppath name */
	if (temppath[0]!='/') {
		rc = EINVAL;
		goto quit;
	}

	/* if the temppath ends with any '/' then remove them */
	len = strlen(temppath);
	while (len > 1 && temppath[len-1] == '/') {
		temppath[--len] = 0;
	}

	/* we start with the file system root minode */
	dir_minode = sys->fsroot;
    ufs_inode_use(dir_minode);  /* bump the use count */
	
	if (dir) *dir = dir_minode;
	if (last) *last = NULL;

	/* does the caller want the full path name returned? */
	if (fullpath) fullpath[0] = 0;

	// wtof("%s:   temppath=\"%s\"", __func__, temppath);
	
	for (name = strtok(temppath, "/"), next = strtok(NULL, "");	name;
		next ? name = strtok(next, "/") : NULL, next = strtok(NULL, "")) 
	{
		// wtof("%s   name=\"%s\"", __func__, name);

		/* does the caller want the full path name returned? */
		if (fullpath) {
			/* Yes, copy it now */
			strcat(fullpath, "/");
			strcat(fullpath, name);
		}

		/* we need 'execute' access to traverse the directory path */
        if (!ufs_inode_auth_exec(acee, dir_minode)) {
            rc = EACCES;
            goto quit;
        }

		/* the dir_minode must be a directory minode */
        if (!ufs_inode_isdir(&dir_minode->dinode)) {
            rc = ENOTDIR;
            goto quit;
        }

		/* If a vdisk is mounted on this minode then we need 
		 * to switch to the mounted vdisk and retrieve the
		 * minode for the root inode on this vdisk. 
		 * 
		 * However, if this is the last minode for this path name
		 * we don't follow the minode->mounted_vdisk to the 
		 * mounted vdisk as we don't know the callers intention
		 * on using the last minode that we return.
		 */
		/* the inode number is located on this vdisk */
		vdisk = dir_minode->vdisk;
		/* this isn't the last minode for this path name */
		if (dir_minode->mounted_vdisk) {
			// wtof("%s:   mount on dir minode=%08X vdisk=%08X ino=%u", 
			// 	__func__, minode, minode->vdisk, minode->inode_number);
			/* switch to mounted vdisk */
			vdisk = dir_minode->mounted_vdisk;
			/* release this minode */
			ufs_inode_rel(dir_minode);
			/* get the minode for the vdisk root inode */
			dir_minode = ufs_inode_get(vdisk, UFS_ROOT_INODE);
			if (!dir_minode) {
				rc = ENOENT;
				goto quit;
			}

			/* we need 'execute' access to traverse the directory path */
			if (!ufs_inode_auth_exec(acee, dir_minode)) {
				rc = EACCES;
				goto quit;
			}

			/* the dir_minode must be a directory minode */
			if (!ufs_inode_isdir(&dir_minode->dinode)) {
				rc = ENOTDIR;
				goto quit;
			}
		}

		if (dir) {
			if (*dir) {
				/* release dir minode we previously saved */
				ufs_inode_rel(*dir);
			}
			/* return dir minode to caller */
			*dir = dir_minode;
		}

		// wtof("%s: looking for \"%s\" in dir_minode=%08X", __func__, name, dir_minode);
		
		/* get the inode number for this name */
        rc = ufs_file_lookup_by_name(dir_minode, name, &ino);
        if (rc) goto quit;
		
		/* we're done using this dir_minode */
		ufs_inode_rel(dir_minode);
		dir_minode = NULL;

		/* get the minode for the vdisk and ino number */
		minode = ufs_inode_get(vdisk, ino);
        if (!minode) {
            rc = ENOENT;
            goto quit;
        }

		/* return results to caller */
        if (next) {
			/* prepare for next iteration */
			dir_minode = minode;
			minode = NULL;
			continue;
		}

		/* this is the last minode */
		if (last) {
			*last = minode;
			minode = NULL;
		}
		break;
	}

	/* if the caller wants the fullpath and it's still empty then we
	 * only processed the root directory minode and no names in the
	 * path, so we correct the fullname with a "/".
	 */
	if (fullpath && fullpath[0]==0) strcpy(fullpath, "/");

quit:
	if (minode) ufs_inode_rel(minode);
	// wtof("%s: exit rc=%d %s", __func__, rc, rc ? strerror(rc) : "SUCCESS");
	// wtof("%s:- - - - - - - - - - - - - - - - - - - - - - - - - - - - -", __func__);
	return rc;
}
