#include "ufs.h"
#include "ufs/file.h"

__asm__("\n&FUNC    SETC 'ufs_open'");
INT32 ufs_open(UFS *ufs, const char *path, BYTE fm, UFSFILE **fpp)
{
    ACEE        *acee       = ufs->acee;
    UFSFILE     *fp         = NULL;
    UFSMIN		*dir_minode = NULL;
    UFSMIN		*last_minode= NULL;
    INT32       rc          = 0;
    char        *p          = NULL;
    char        fullpath[UFS_PATH_MAX+1] = "";
    char        name[UFS_NAME_MAX+1] = "";

    *fpp = NULL;

    /* check for valid open mode (must have at least read or write) */
    if (!(fm & (FILE_MODE_READ + FILE_MODE_WRITE))) {
		rc = EINVAL;
		goto quit;
	}

    /* locate inode for file in path */
    rc = ufs_lookup_minode(ufs, path, &dir_minode, &last_minode, fullpath);
    if (rc) goto quit;  /* file doesn't exist or no access to file */

    /* path name exist */
    p = strrchr(fullpath, '/');
    if (p) {
        /* extract the file name from the full path name */
        strcpyp(name, UFS_NAME_MAX, p+1, 0);
        name[UFS_NAME_MAX]=0;
    }

    /* if we're opening for write access, make sure disk is not read only */
    if (fm & FILE_MODE_WRITE && last_minode->vdisk->disk->readonly) {
		rc = EROFS;
		goto quit;
	}

    /* this shouldn't happen but just in case, do we have a last_minode? */
    if (!last_minode) {
		rc = ENOENT;
		goto quit;
	}

    /* make sure this is a file inode */
    if (ufs_inode_isdir(&last_minode->dinode)) {
		rc = EISDIR;  /* attempt to open directory */
		goto quit;
	}
    if (!ufs_inode_isfile(&last_minode->dinode)) {
		rc = ENFILE; /* not a file, yikes! */
		goto quit;
	}

    /* check file access permission */
    if (fm & FILE_MODE_READ) {
        if (!ufs_inode_auth_read(acee, last_minode)) {
			rc = EACCES;
			goto quit;
		}
    }
    if (fm & FILE_MODE_WRITE) {
        if (!ufs_inode_auth_write(acee, last_minode)) {
			rc = EACCES;
			goto quit;
		}
    }

    /* allocate a new file handle */
    fp = ufs_file_new(ufs->sys, acee, last_minode);
    if (!fp) {
		rc = ENOMEM;
		goto quit;
	}
	/* success, last_minode is now part of the file handle */
    last_minode = NULL;

    /* copy file mode to file handle */
    fp->mode    = fm;

    /* copy file name to handle */
    strcpyp(fp->name, sizeof(fp->name), name, 0);

    /* update access time */
    fp->minode->dinode.atime.v2 = mtime64(NULL);

    if (fp->mode & FILE_MODE_WRITE) {
        /* update modified time */
        fp->minode->dinode.mtime  = fp->minode->dinode.atime;
    }

    if (fm & FILE_MODE_TRUNC) {
        /* truncate file */
        ufs_datablock_size_change(fp->vdisk, 0, fp->minode);
    }

    if (fm & FILE_MODE_APPEND) {
        /* position to end of file */
        ufs_fseek(fp, 0, UFS_SEEK_END);
    }
    else {
        /* position to start of file */
        ufs_fseek(fp, 0, UFS_SEEK_SET);
    }

    /* update the file inode */
    ufs_inode_writeback(fp->minode);
    fp->minode->flags &= ~MINODE_FLAG_DIRTY;

    /* return file handle to caller */
    *fpp = fp;

quit:
    if (last_minode) ufs_inode_rel(last_minode);
    if (dir_minode) ufs_inode_rel(dir_minode);
    errno = rc;
    return rc;
}


#if 0 /* old code */
__asm__("\n&FUNC    SETC 'ufs_open'");
INT32 ufs_open(UFS *ufs, const char *path, BYTE fm, UFSFILE **fpp)
{
    UFSSYS      *sys        = ufs->sys;
    UFSCWD      *cwd        = ufs->cwd;
    ACEE        *acee       = ufs->acee;
    UFSFILE     *fp         = NULL;
    UFSMIN      *minode     = NULL;
    UFSVDISK    *vdisk      = NULL;
    UINT32      ino         = 0;
    INT32       rc          = 0;
    char        *p          = NULL;
    char        fullpath[UFS_PATH_MAX+1] = "";
    char        name[UFS_NAME_MAX+1] = "";

    *fpp = NULL;

    /* check for valid open mode (must have at least read or write) */
    if (!(fm & (FILE_MODE_READ + FILE_MODE_WRITE))) return EINVAL;

    /* locate inode for file in path */
    rc = ufs_lookup(ufs, UFS_LOOKUP, path, &vdisk, &ino, fullpath);
    if (rc) return rc;  /* file doesn't exist or no access to file */

    /* path name exist */
    p = strrchr(fullpath, '/');
    if (p) {
        /* extract the file name from the full path name */
        strcpyp(name, UFS_NAME_MAX, p+1, 0);
        name[UFS_NAME_MAX]=0;
    }

    /* if we're opening for write access, make sure disk is not read only */
    if (fm & FILE_MODE_WRITE && vdisk->disk->readonly) return EROFS;

    /* get minode for this inode number */
    minode = ufs_inode_get(vdisk, ino);
    if (!minode) return ENOENT;

    /* make sure this is a file inode */
    if (ufs_inode_isdir(&minode->dinode)) goto quit_is_dir;     /* attempt to open directory */
    if (!ufs_inode_isfile(&minode->dinode)) goto quit_not_file; /* not a file, yikes! */

    /* check file access permission */
    if (fm & FILE_MODE_READ) {
        if (!ufs_inode_auth_read(acee, minode)) goto quit_perm;
    }
    if (fm & FILE_MODE_WRITE) {
        if (!ufs_inode_auth_write(acee, minode)) goto quit_perm;
    }

    /* allocate a new file handle */
    fp = ufs_file_new(sys, acee, minode);
    if (!fp) goto quit_nomem;
    minode      = NULL;

    /* copy file mode to file handle */
    fp->mode    = fm;

    /* copy file name to handle */
    strcpyp(fp->name, sizeof(fp->name), name, 0);

    /* update access time */
    fp->minode->dinode.atime.v2 = mtime64(NULL);

    if (fp->mode & FILE_MODE_WRITE) {
        /* update modified time */
        fp->minode->dinode.mtime  = fp->minode->dinode.atime;
    }

    if (fm & FILE_MODE_TRUNC) {
        /* truncate file */
        ufs_datablock_size_change(vdisk, 0, fp->minode);
    }

    if (fm & FILE_MODE_APPEND) {
        /* position to end of file */
        ufs_fseek(fp, 0, UFS_SEEK_END);
    }
    else {
        /* position to start of file */
        ufs_fseek(fp, 0, UFS_SEEK_SET);
    }

    /* update the file inode */
    ufs_inode_writeback(fp->minode);
    fp->minode->flags &= ~MINODE_FLAG_DIRTY;

    /* return file handle to caller */
    *fpp = fp;
    return 0;

quit_perm:
    rc = EPERM;
    goto quit;

quit_is_dir:
    rc = EISDIR;
    goto quit;

quit_not_file:
    rc = ENFILE;
    goto quit;

quit_nomem:
    if (fp) ufs_file_free(&fp);
    rc = ENOMEM;

quit:
    if (minode) ufs_inode_rel(minode);
    errno = rc;
    return rc;
}
#endif /* old code */
