#ifndef UFS_MOUNT_H
#define UFS_MOUNT_H

#include "ufs/sys.h"
#include "ufs/inode.h"

/* ufs_mount() mount a ddname on directory path.
 * note that the requested ddname must have already been opened
 * by ufs_sys_new() and the path directory must already exist.
 */
INT32 ufs_mount(UFS *ufs, const char *ddname, const char *path)			asm("UFSMOUNT");

/* ufs_fstab() - reads file fn and mounts disk to paths specified in the file */
/* the fn should be a "/etc/fstab" text file */
/* note that ufs_sys_new() will call ufs_fstab() with "/etc/fstab" after opening disk */
int ufs_fstab(UFS *ufs, const char *fn)									asm("UFSFSTAB");


/* ufs_unmount_inode() - un mounts a minode mounted vdisk */
int ufs_unmount_inode(UFS *ufs, UFSMIN *minode)							asm("UFSUNMIN");

/* ufs_unmount_path() un mounts disk from path name */
int ufs_unmount_path(UFS *ufs, const char *path)						asm("UFSUNMPA");

/* ufs_unmount_ddname() un mount disk associated with ddname */
int ufs_unmount_ddname(UFS *ufs, const char *ddname)					asm("UFSUNMDD");

/* unmoount all disk from file system */
int ufs_unmount_all(void)												asm("UFSUNMAL");

#endif
