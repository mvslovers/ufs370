#include "ufs/sys.h"
#include "ufs/file.h"

static void fix_path(char *fullpath);

__asm__("\n&FUNC    SETC 'ufs_lookup'");
INT32 ufs_lookup(UFS *ufs, UFSLKOP op, const char *path, UFSVDISK **vdisk, UINT32 *result, char *fullpath)
{
    UFSSYS      *sys    = ufs->sys;
    UFSCWD      *cwd    = ufs->cwd;
    ACEE        *acee   = ufs->acee;
    UINT32      ino     = 0;
    INT32       islast  = 0;
    INT32       rc      = 0;
    INT32       len     = 0;
    UFSMIN      *minode;
    INT32       namelen;
    char        *p;
    const char  *name;
    const char  *nextname;
    char        inpath[UFS_PATH_MAX+1];
#if 0
    wtof("%s enter, path=%08X, vdisk=%08X, result=%08X, fullpath=%08X",
         __func__, path, vdisk, result, fullpath);
#endif
    if (result) {
        *result = 0;
        /* wtof("%s *result=%u", __func__, *result); */
    }

    if (!path) path = "/";

    /* wtof("%s #1", __func__); */
    if (fullpath) {
        strcpyp(fullpath, UFS_PATH_MAX, "/", 0);
        len = 1;
    }

    /* wtof("%s #2", __func__); */
    if (path[0]=='/') {
        /* wtof("%s #3", __func__); */
        /* path starts with root directory */
        minode = cwd->root_dir;
        while(*path=='/') path++;
    }
    else {
        /* wtof("%s #4", __func__); */
        /* path starts with current directory */
        minode = cwd->cur_dir;
        if (cwd->path[0]) {
            if (fullpath) {
                strcpyp(fullpath, UFS_PATH_MAX, cwd->path, 0);
                len = strlen(fullpath);
            }
        }
    }

    /* wtof("%s #5", __func__); */
    if (!minode) minode = sys->fsroot;  /* we've got to start some place :-) */
    /* wtof("%s minode=%08X", __func__, minode); */

    ufs_inode_use(minode);  /* bump the use count */
    *vdisk = minode->vdisk;
    /* wtof("%s *vdisk=%08X", __func__, *vdisk); */

    strcpyp(inpath, UFS_PATH_MAX, (char*)path, 0);
    inpath[sizeof(inpath)-1]=0;
    /* wtof("%s inpath=\"%s\"", __func__, inpath); */

    name = strtok(inpath, "/");
    nextname = name ? strtok(NULL, "/") : NULL;
    islast = nextname ? 0 : 1;

    /* wtof("%s name=\"%s\", nextname=\"%s\", islast=%d", __func__, name, nextname, islast); */
    while(name) {
        namelen = strlen(name);

        /* get the vdisk handle for this inode */
        *vdisk = minode->vdisk;

#if 0	/* debugging */
        if (minode) {
			wtof("%s:- - - - - - - - - - - - - - - - - - - - - - - - -", __func__);
			wtof("%s: name=\"%s\"", __func__, name);
			wtodumpf(*vdisk, sizeof(UFSVDISK), "%s: current vdisk", __func__);
			wtodumpf((*vdisk)->disk, sizeof(UFSDISK), "%s: current disk", __func__);
			wtodumpf(minode, sizeof(UFSMIN), "%s: current minode", __func__);
			wtof("%s:- - - - - - - - - - - - - - - - - - - - - - - - -", __func__);
        }
#endif

        /* wtof("%s inode_number=%u path=\"%s\"", __func__, parent->inode_number, path); */
        if (!ufs_inode_auth_exec(acee, minode)) {
            /* wtof("%s user does not have %s access to directory", __func__, "EXEC"); */
            rc = EPERM;
            break;
        }

        if (islast) {
            /* name is last name in path */
            if ((*vdisk)->disk->readonly) {
                /* this disk is mounted read only */
                if (op == UFS_DELETE || op == UFS_RENAME) {
                    rc = EROFS;
                    break;
                }
            }
            /* check for "." rename */
            if (namelen == 1 && name[0] == '.') {
                if (op == UFS_RENAME) {
                    /* can't rename "." */
                    rc = EISDIR;
                    break;
                }
            }
        }

        /* No lookup on removed directory */
        if (ufs_inode_nlink(&minode->dinode) == 0) {
            rc = ENOENT;
            break;
        }

        /* convert name to inode number */
        /* wtof("%s ufs_file_lookup_by_name(%s)", __func__, name); */
        rc = ufs_file_lookup_by_name(minode, name, &ino);
        /* wtof("%s rc=%d", __func__, rc); */
        if (rc) {
            /* not found */
            if (islast) {
                /* name is last in path */
                if ((op == UFS_CREATE) || (op == UFS_RENAME)) {
                    /* check directory permission to allocate. */
                    if (!ufs_inode_auth_write(acee, minode)) {
                        /* wtof("%s user does not have %s access to directory", __func__, "WRITE"); */
                        rc = EPERM;
                    }
                }
            }
            break;  /* return error */
        }

        /* rc==0, name exist in directory */
        if (fullpath) {
            if (fullpath[len-1] != '/' && len < UFS_PATH_MAX) {
                fullpath[len++] = '/';
            }

            if (len + namelen < UFS_PATH_MAX) {
                memcpy(&fullpath[len], name, namelen);
                len += namelen;
            }
            else {
                rc = E2BIG;
                break;
            }

            fullpath[len] = 0;
        }

        if (islast && (op == UFS_DELETE)) {
            /* make sure caller has write access to directory */
            if (!ufs_inode_auth_write(acee, minode)) {
                /* wtof("%s user does not have %s access to directory", __func__, "WRITE"); */
                rc = EPERM;
                break;
            }
        }

        /* release the inode we started with */
        *vdisk = minode->vdisk;
        ufs_inode_rel(minode);

        /* get the inode by its inode number */
        /* wtof("%s ufs_inode_get()", __func__); */
        minode = ufs_inode_get(*vdisk, ino);
        /* wtof("%s minode=%08X", __func__, minode); */
        if (!minode) {
            rc = ENOENT;  /* that didn't work, Yikes! */
            break;
        }

        if (islast) break;

        /* name was NOT last name in path */
#if 1
        /* process mount point on directories */
        if (ufs_inode_isdir(&minode->dinode)) {
            /* check for mount point on this inode */
            if (minode->mounted_vdisk) {
                /* follow mount point to mounted disk */
                *vdisk = minode->mounted_vdisk;
                /* release the current inode */
                ufs_inode_rel(minode);
                /* get the root inode on the mounted disk */
                minode = ufs_inode_get(*vdisk, UFS_ROOT_INODE);
                if (!minode) {
                    rc = ENOENT;
                    break;
                }
            }
        }
#endif

        /* make sure this inode is a directory inode */
        if (!ufs_inode_isdir(&minode->dinode)) {
            rc = ENOTDIR;
            /* wtof("%s ufs_inode_isdir() FALSE, rc=%d", __func__, rc); */
            break;
        }

        name = nextname;
        if (!name) break;

        nextname = strtok(NULL, "/");
        islast = nextname ? 0 : 1;

        /* wtof("%s name=\"%s\", nextname=\"%s\", islast=%d", __func__, name, nextname, islast); */
    }

quit:
    /* wtof("%s minode=%08X", __func__, minode); */
    if (minode) {
        *vdisk = minode->vdisk;
        if (result) *result = minode->inode_number;
        ufs_inode_rel(minode);
    }

    if (fullpath) {
        /* wtof("%s fix_path(\"%s\")", __func__, fullpath); */
        fix_path(fullpath);

        /* wtof("%s fullpath=\"%s\"", __func__, fullpath); */
    }

    return rc;
}

__asm__("\n&FUNC    SETC 'fix_path'");
static void fix_path(char *fullpath)
{
    char    *p;
    int     rc;

    /* translate "//", "/./" and "/../" path name components */
    do {
        char *p1=0, *p2=0;
        rc = 1;
        /* wtof("%s fullpath=\"%s\"", __func__, fullpath); */
        for(p=fullpath; *p; p++) {
            if (p[0]=='/') {
                p2 = p1;
                p1 = p;
                if (p[1]==0 && p > fullpath) {
                    /* path ends with "/", "/name/" -> "/name" */
                    p[0]=0;
                    break;
                }
                if (p[1]=='/') {
                    /* "//" -> "/" */
                    strcpy(p1, &p[1]);
                    rc = 0;
                    break;
                }
                if (p[1]=='.') {
                    /* "/." */
                    if (p[2]=='/') {
                        /* "/./" -> "/" */
                        strcpy(p1, &p[2]);
                        rc = 0;
                        break;
                    }
                    if (p[2]==0) {
                        /* "/name/." -> "/name" */
                        if (p2) {
                            p[0] = 0;
                        }
                        else {
                            p[1] = 0;
                        }
                        rc = 0;
                        break;
                    }
                    if (p[2]=='.') {
                        /* "/.." */
                        if (p[3]=='/') {
                            /* "/../" -> prev "/" */
                            strcpy(p2?p2:p1, &p[3]);
                            rc = 0;
                            break;
                        }
                        if (p[3]==0) {
                            /* "/name/.." -> "/" */
                            if (p2) {
                                p2[1]=0;
                            }
                            else {
                                p1[1]=0;
                            }
                            rc = 0;
                            break;
                        }
                    }
                }
            }
        }
    } while(rc==0);
}
