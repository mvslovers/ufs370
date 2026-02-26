#ifndef UFS_H
#define UFS_H

typedef struct ufs      UFS;

#include "ufs/types.h"
#include "ufs/sys.h"
#include "ufs/cwd.h"
#include "ufs/mount.h"
#include <racf.h>

#define UFS_EOF     (-1)

/* ufsnew() allocate a UFS handle */
UFS  *ufsnew(void);

/* ufs_get_sys() returns UFSSYS handle from UFS handle */
UFSSYS *ufs_get_sys(UFS *ufs)                                           asm("UFS#GSYS");

/* ufs_set_sys() sets UFSSYS handle into UFS handle */
UFSSYS *ufs_set_sys(UFS *ufs, UFSSYS *sys)                              asm("UFS#SSYS");

/* ufs_get_cwd() returns UFSCWD handle from UFS handle */
UFSCWD *ufs_get_cwd(UFS *ufs)                                           asm("UFS#GCWD");

/* ufs_set_cwd() sets UFSCWD handle into UFS handle */
UFSCWD *ufs_set_cwd(UFS *ufs, UFSCWD *cwd)                              asm("UFS#SCWD");

/* ufs_get_acee() returns ACEE handle from UFS handle */
ACEE   *ufs_get_acee(UFS *ufs)                                          asm("UFS#GACE");

/* ufs_set_acee() sets ACEE handle into UFS handle */
ACEE   *ufs_set_acee(UFS *ufs, ACEE *acee)                              asm("UFS#SACE");

/* ufs_get_create_perm() get the file/directory creation permission value (0755 default) */
UINT32 ufs_get_create_perm(UFS *ufs)                                    asm("UFS#GCRE");

/* ufs_set_create_perm() set the file/directory creation permission value, returns previous value */
UINT32 ufs_set_create_perm(UFS *ufs, UINT32 perm)                       asm("UFS#SCRE");

/* ufsfree() release UFS handle allocated by ufsnew() */
void ufsfree(UFS **ufs);


/* ufs_signon() obtain ACEE handle for userid/password and save ACEE into UFS handle */
int ufs_signon(UFS *ufs, const char *userid, const char *password, const char *group) asm("UFS#SON");

/* ufs_signoff() release ACEE handle obtained by ufs_signon(), removes ACEE from UFS handle */
void ufs_signoff(UFS *ufs)                                              asm("UFS#SOFF");

/* ufs_chgdir() change directory */
int ufs_chgdir(UFS *ufs, const char *path)                              asm("UFS#CD");

/* ufs_mkdir() Create directory */
int ufs_mkdir(UFS *ufs, const char *path)                               asm("UFS#MD");

/* ufs_rmdir() Remove directory */
int ufs_rmdir(UFS *ufs, const char *path)                               asm("UFS#RD");

/* ufs_remove() Remove file */
int ufs_remove(UFS *ufs, const char *path)                              asm("UFS#REM");

/* ufs_diropen() Open directory for reading */
UFSDDESC *ufs_diropen(UFS *ufs, const char *path, const char *pattern)  asm("UFS#DOPN");

/* ufs_dirread() Read next directory record */
UFSDLIST *ufs_dirread(UFSDDESC *ddesc)                                  asm("UFS#DRD");

/* ufs_dirclose() Close directory handle */
void ufs_dirclose(UFSDDESC **ppddsec)                                   asm("UFS#DCLS");

/* ufs_sync() sync file system buffers to disk */
void ufs_sync(UFS *ufs)                                                 asm("UFS#SYNC");


/* ufs_fopen() open file */
UFSFILE *ufs_fopen(UFS *ufs, const char *path, const char *mode)        asm("UFS#FOPN");

/* ufs_open() open existing file */
INT32 ufs_open(UFS *ufs, const char *path, BYTE fm, UFSFILE **fp)       asm("UFS#OPEN");

/* ufs_create() create new file */
INT32 ufs_create(UFS *ufs, const char *path, BYTE fm, UFSFILE **fp)     asm("UFS#CRTE");

/* ufs_fclose() close file */
void ufs_fclose(UFSFILE **file)                                         asm("UFS#FCLS");

/* ufs_fsync() sync file handle to disk */
void ufs_fsync(UFSFILE *file)                                           asm("UFS#FSYN");

/* ufs_clearerr() clear error indicator */
void ufs_clearerr(UFSFILE *file)                                        asm("UFS#CERR");

/* ufs_ferror() return file handle error or 0 if no error */
INT32 ufs_ferror(UFSFILE *file)                                         asm("UFS#FERR");

/* ufs_feof() returns true if file position is past end of file */
INT32 ufs_feof(UFSFILE *file)                                           asm("UFS#FEOF");

/* ufs_fgetc() returns next character from stream or -1 for EOF, -2 for Error */
INT32 ufs_fgetc(UFSFILE *file)                                          asm("UFS#FGTC");

/* ufs_fputc() write character to file, returns EOF on error */
INT32 ufs_fputc(INT32 c, UFSFILE *file)                                 asm("UFS#FPTC");

/* ufs_fputs() write string to file, returns EOF on error */
INT32 ufs_fputs(const char *str, UFSFILE *file)                         asm("UFS#FPTS");

/* ufs_fread() read nitems of size from file, returns items read count */
UINT32 ufs_fread(void *ptr, UINT32 size, UINT32 nitems, UFSFILE *fp)    asm("UFS#FRD");

/* ufs_fwrite() write nitems of size to file, returns items written count */
UINT32 ufs_fwrite(void *ptr, UINT32 size, UINT32 nitems, UFSFILE *fp)   asm("UFS#FWT");

/* ufs_fgets() read characters from file stream until newline found */
char *ufs_fgets(char *str, int num, UFSFILE *fp)                        asm("UFS#FGTS");

/* ufs_fseek() seek to offset relative to whence */
INT32 ufs_fseek(UFSFILE *fp, INT32 offset, INT32 whence)                asm("UFS#FSEE");
#define UFS_SEEK_SET    0   /* from beginning of file   */
#define UFS_SEEK_CUR    1   /* ftom current position in file */
#define UFS_SEEK_END    2   /* from end of file */

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 * The following are not part of the API for UFS and are used internally
 * by several of the API functions above.
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */

/* ufs_lookup() locate/find inode, inode returned in result, full path name returned in fullpath */
typedef enum ufslkop {
    UFS_LOOKUP,     /* lookup, last path name is expected to be found */
    UFS_CREATE,     /* lookup, last path name is not expected to be found */
    UFS_DELETE,     /* lookup, last path name is expected to be found */
    UFS_RENAME      /* lookup, last path name is expected to be found */
} UFSLKOP;
INT32 ufs_lookup(UFS *ufs, UFSLKOP op, const char *path, UFSVDISK **vdisk, UINT32 *ino, char *fullpath) asm("UFS#LKUP");

/* ufs_lookup_minode() - retrieve minode of directory and last path name */
int ufs_lookup_minode(UFS *ufs, const char *path, UFSMIN **dir, UFSMIN **last, char *fullpath)	asm("UFS#LMIN");
/* The UFSMIN pointers returned have already had the use count incremented
 * and should be safe to use.  The caller must call ufs_inode_rel() for
 * any UFSMIN pointers returned to the caller.
 */

/* ufs_inode_mounted_vdisk() - returns UFSMIN for a mounted vdisk */
UFSMIN *ufs_inode_mounted_vdisk(UFS *ufs, UFSVDISK *vdisk)							asm("UFSINMVD");


#endif /* UFS_H */
