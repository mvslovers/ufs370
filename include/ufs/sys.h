#ifndef UFS_SYS_H
#define UFS_SYS_H

#include <clibwto.h>
#include <racf.h>

#include "ufs/types.h"      /* atomic types                             */
#include "ufs/time.h"       /* time functions                           */
#include "ufs/disk.h"       /* low level disk, boot block, super block  */
#include "ufs/pager.h"      /* block pager handles                      */
#include "ufs/vdisk.h"      /* virtual disk                             */
#include "ufs/inode.h"      /* index nodes                              */
#include "ufs/dir.h"        /* directory entry                          */
#include "ufs/dev.h"        /* device handles                           */
#include "ufs/name.h"       /* name to vdisk                            */
#include "ufs/cwd.h"        /* current working directory                */

typedef struct ufs_sys  UFSSYS;
struct ufs_sys {
    char        eye[8];                 /* 00 eye catcher for dumps             */
#define UFSSYS_EYE  "*UFSSYS*"          /* ... eye catcher for dumps            */
    UFSDISK     **disks;                /* 08 array of disk handles             */
    UFSPAGER    **pagers;               /* 0C array of pager handles            */
    UFSIO       **io;                   /* 10 array of io handles               */
    UFSVDISK    **vdisks;               /* 14 array of virtual disk handles     */
    UFSDEV      **devs;                 /* 18 array of devices                  */
    UINT32      next_dvnum;             /* 1C next device number to assign      */
    UFSMIN      *fsroot;                /* 20 file system root inode "/"        */
    UFSNAME     **names;                /* 24 array of name to vdisk            */
    UFSCWD      **cwds;                 /* 28 array of current working directory*/
    UFSFILE     **files;                /* 2C opened file descriptors           */
    UFSMIN      **mountpoint;           /* 30 array directories with mounted vdisk */
};                                      /* 40 (64 bytes)                        */

UFSSYS *ufs_sys_get(void)                       asm("UFSSYGET");

UFSSYS *ufs_sys_new(void)                       asm("UFSSYNEW");

void ufs_sys_term(void)                         asm("UFSSYTRM");

/* Private "user" handle */
struct ufs {
    char        eye[8];             /* 00 eye catcher                       */
#define UFSEYE  "**UFS**"           /* ... eye catcher                      */
    UFSSYS      *sys;               /* 08 system handle                     */
    UFSCWD      *cwd;               /* 0C Current working directory         */
    ACEE        *acee;              /* 10 user security handle              */
    UINT32      flags;              /* 14 flags                             */
#define UFS_ACEE_DEFAULT    0x80000000  /* address space acee handle        */
#define UFS_ACEE_USER       0x40000000  /* user set acee handle             */
#define UFS_ACEE_SIGNON     0x20000000  /* ufs_signon() acee handle         */
    UINT32      create_perm;        /* 18 file creation permission          */
    UINT32      unused;             /* 1C unused                            */
};                                  /* 20 (32 bytes)                        */

#include "ufs.h"            /* Private "user" functions                 */

#endif /* UFS_SYS_H */
