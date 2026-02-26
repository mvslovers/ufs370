#include "ufs/cwd.h"
#include "ufs/sys.h"

__asm__("\n&FUNC    SETC 'ufs_cwd_new'");
UFSCWD  *ufs_cwd_new(void)
{
    UFSCWD  *cwd = calloc(1, sizeof(UFSCWD));
    UFSSYS  *sys = ufs_sys_get();
    INT32   lockrc;
    INT32   lockfsroot;

    lockrc = lock(sys, 0);

    if (cwd) {
        cwd->refcnt++;

        if (sys->fsroot) {
            cwd->root_dir = ufs_inode_use(sys->fsroot);
            cwd->cur_dir  = ufs_inode_use(sys->fsroot);
            strcpy(cwd->path, "/");
        }

        array_add(&sys->cwds, cwd);
    }

quit:
   if (lockrc==0) unlock(sys,0);
    return cwd;
}
