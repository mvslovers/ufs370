#include "ufs/sys.h"
#include "ufs/file.h"

__asm__("\n&FUNC    SETC 'ufs_dirclose'");
void ufs_dirclose(UFSDDESC **ppddesc)
{
    UFSDDESC    *desc;
    UFSDLIST    *list;
    UINT32      count;
    UINT32      n;

    if (ppddesc && *ppddesc) {
        desc = *ppddesc;

        if (desc->dlist) {
            count = array_count(&desc->dlist);

            for(n=0; n < count; n++) {
                list = desc->dlist[n];
                if (!list) continue;

                free(list);
                desc->dlist[n] = NULL;
            }

            array_free(&desc->dlist);
        }

        if (desc->minode) ufs_inode_rel(desc->minode);

        free(desc);
        *ppddesc = 0;
    }
}
