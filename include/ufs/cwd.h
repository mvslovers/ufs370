#ifndef UFS_CWD_H
#define UFS_CWD_H
#include "ufs/types.h"      /* atomic types                             */
#include "ufs/time.h"       /* time functions                           */
#include "ufs/disk.h"       /* low level disk, boot block, super block  */
#include "ufs/inode.h"      /* index nodes                              */

struct ufs_cwd {
    UFSMIN      *cur_dir;       /* 00 current directory                 */
    UFSMIN      *root_dir;      /* 04 root directory                    */
    UINT32      unused;         /* 08 unused                            */
    UINT32      refcnt;         /* 0C reference count                   */
    char        path[256];      /* 10 current directory string          */
};                              /* 110  (272 bytes)                     */

UFSCWD  *ufs_cwd_new(void)                              asm("UFSCWDNW");
void    ufs_cwd_free(UFSCWD **cwd)                      asm("UFSCWDFR");

#endif /* UFS_CWD_H */
