#include "ufs/cwd.h"
#include "ufs/sys.h"

__asm__("\n&FUNC    SETC 'ufs_cwd_free'");
void ufs_cwd_free(UFSCWD **cwd)
{
    UFSCWD  *c      = cwd ? *cwd : NULL;
    UFSSYS  *sys    = ufs_sys_get();
    INT32   lockrc  = -1;
    INT32   lockc   = -1;
    UINT32  count;
    UINT32  n;

    if (!c) return;

    lockrc = lock(sys, 0);

    lockc = lock(c, 0);
    if (c->refcnt > 0) c->refcnt--;

    if (c->refcnt!=0) goto quit;

    count = array_count(&sys->cwds);
    for(n=count; n > 0; n--) {
        if (c == array_get(&sys->cwds, n)) {
            array_del(&sys->cwds, n);
            break;
        }
    }

    if (c->cur_dir) {
        ufs_inode_rel(c->cur_dir);
        c->cur_dir = NULL;
    }

    if (c->root_dir) {
        ufs_inode_rel(c->root_dir);
        c->root_dir = NULL;
    }

    free(c);
    *cwd = NULL;

quit:
    if (lockc==0)   unlock(c,0);
    if (lockrc==0)  unlock(sys,0);
    return;
}
