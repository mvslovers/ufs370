#include "ufs.h"
#include "ufs/file.h"

__asm__("\n&FUNC    SETC 'ufs_create'");
INT32 ufs_create(UFS *ufs, const char *path, BYTE fm, UFSFILE **fpp)
{
    UFSSYS      *sys        = ufs->sys;
    UFSCWD      *cwd        = ufs->cwd;
    ACEE        *acee       = ufs->acee;
    UFSFILE     *fp         = NULL;
    UFSMIN      *minode     = NULL;
    UFSVDISK    *vdisk      = NULL;
    UINT32      ino         = 0;
    INT32       rc          = 0;
    const char  *p          = NULL;
    UFSATTR     attr        = {0};
    char        owner[9]    = "";
    char        group[9]    = "";
    char        fullpath[UFS_PATH_MAX+1] = "";
    char        name[UFS_NAME_MAX+1] = "";

    *fpp = NULL;

    /* check for valid open mode (must have at least write) */
    if (!(fm & FILE_MODE_WRITE)) return EINVAL;

    /* locate inode for file in path */
    rc = ufs_lookup(ufs, UFS_CREATE, path, &vdisk, &ino, fullpath);
    if (rc == 0) return EEXIST;
    if (rc != ENOENT) return rc;

    /* ENOENT - file does not exist *AND* we have write permission on the parent dir */

    /* make sure disk is not read only */
    if (vdisk->disk->readonly) return EROFS;

    /* extract the file name from the path */
    p = strrchr(path, '/');
    if (p) {
        p++;
    }
    else {
        p = path;
    }
    strcpyp(name, UFS_NAME_MAX, (char*)p, 0);
    name[UFS_NAME_MAX]=0;

    /* get inode of parent directory */
    minode = ufs_inode_get(vdisk, ino);
    if (!minode) return ENOENT;

    /* make sure parent inode is a directory */
    if (!(ufs_inode_isdir(&minode->dinode))) goto quit_not_dir;

    /* allocate new file in parent directory */
    if (acee && memcmp(acee->aceeacee, "ACEE", 4)==0) {
        /* get owner and group from security environment */
        memcpyp(owner, sizeof(owner), &acee->aceeuser[1], acee->aceeuser[0], 0);
        memcpyp(group, sizeof(group), &acee->aceegrp[1], acee->aceegrp[0], 0);
    }
    else {
        /* plug in some *safe* defaults */
        strcpy(owner, "HERC01");
        strcpy(group, "STGADMIN");
    }
    attr.owner  = owner;
    attr.group  = group;
    attr.mode   = UFS_IFREG | ufs->create_perm;
#if 0
    attr.atime  = ufs_timeval(NULL);
#else
	attr.atime.v2 = mtime64(NULL);
#endif
    attr.mtime  = attr.atime;
    attr.ctime  = attr.atime;

    /* allocate new file */
    rc = ufs_file_allocate(minode, path, &attr, &ino);
    /* wtof("%s rc=%d, ino=%u", __func__, rc, ino); */
    if (rc) goto quit;

    /* release the inode of the parent directory */
    ufs_inode_rel(minode);

    /* get inode for this file */
    minode = ufs_inode_get(vdisk, ino);
    if (!minode) return ENOENT;
    /* wtodumpf(minode, sizeof(UFSMIN), "%s Child=%u", __func__, ino); */

    /* allocate a new file handle */
    fp = ufs_file_new(sys, acee, minode);
    if (!fp) goto quit_nomem;
    minode      = NULL;

    /* copy file mode to file handle */
    fp->mode    = fm;

    /* copy file name to handle */
    strcpyp(fp->name, sizeof(fp->name), name, 0);

    /* allocate a disk buffer for our file handle */
    fp->blkbuf  = ufs_disk_get_buf(vdisk->disk);
    if (!fp->blkbuf) goto quit_nomem;

    /* position to start of file */
    ufs_fseek(fp, 0, UFS_SEEK_SET);

    /* return file handle to caller */
    *fpp = fp;
    return 0;

quit_perm:
    rc = EPERM;
    goto quit;

quit_not_dir:
    rc = ENOTDIR;
    goto quit;

quit_nomem:
    if (fp) ufs_file_free(&fp);
    rc = ENOMEM;

quit:
    if (minode) ufs_inode_rel(minode);
    errno = rc;
    return rc;
}
