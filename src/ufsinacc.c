#include <clibcrt.h>
#include <racf.h>
#include "ufs/inode.h"

static __inline BOOL access_allowed(UINT32 ufsacc, UINT32 mode)
{
    return (ufsacc & mode) ? 1 : 0;
}

__asm__("\n&FUNC    SETC 'ufs_inode_auth'");
BOOL ufs_inode_auth(ACEE *acee, UFSMIN *minode, UINT32 ufsacc)
{
    UINT32  mode;
    INT32   len;
    CONNGRP *grp;

    /* Note: the 16 bit inode mode value has access bits in the lower 9 bits
    ** (Owner=3, Group=3, World=3) with each 3 bit being RWX (Read, Write, Execute)
    ** 1 = Execute
    ** 2 = Write
    ** 3 = Write and Execute
    ** 4 = Read
    ** 5 = Read and Execute
    ** 6 = Read and Write
    ** 7 = Read, Write and Execute
    */
    /* wtof("%s ACEE=%08X, ufsacc=%u", __func__, acee, ufsacc); */

    /* check for WORLD access first since we don't need any credentials for this test */
    mode = minode->dinode.mode & 7;
    if (access_allowed(ufsacc, mode)) return 1; /* access allowed */

#if 0
    /* we've disabled this code to prevent WORLD users (no ACEE) */
    /* from gaining access provided by the default environment (server) */

    /* all other test require credentials via an ACEE structure */
    if (!acee) {
        /* get ACEE from thread/task environment */
        CLIBCRT *crt = __crtget();
        if (crt) acee = crt->crtacee;
    }

    if (!acee) {
        /* get ACEE from address space */
        acee = racf_get_acee();
    }
#endif
    /* all other test require credentials via an ACEE structure */
    if (!acee) {
        /* we have nothing to go on */
        return 0;   /* access NOT allowed */
    }

    /* check for OWNER access */
    len = strlen(minode->dinode.owner);
    mode = (minode->dinode.mode >> 6) & 7;
    if (len==acee->aceeuser[0]) {
        if (strncmpi(minode->dinode.owner, &acee->aceeuser[1], len)==0) {
            if (access_allowed(ufsacc, mode)) return 1; /* access allowed */
        }
    }
    /* check for GROUP access */
    len = strlen(minode->dinode.group);
    mode = (minode->dinode.mode >> 3) & 7;
    if (len==acee->aceegrp[0]) {
        if (strncmpi(minode->dinode.group, &acee->aceegrp[1], len)==0) {
            if (access_allowed(ufsacc, mode)) return 1; /* access allowed */
        }
    }

    /* check for connected GROUP access */
    for( grp=acee->aceecgrp; grp; grp=grp->next ) {
        if (len==grp->group[0]) {
            if (strncmpi(minode->dinode.group, &grp->group[1], len)==0) {
                if (access_allowed(ufsacc, mode)) return 1;    /* access allowed */
            }
        }
    }

    return 0;   /* access NOT allowed */
}

#if 0
__asm__("\n&FUNC    SETC 'access_allowed'");
static BOOL access_allowed(UINT32 ufsacc, UINT32 mode)
{
    BOOL    rc = 0; /* access NOT allowed */

    if (ufsacc & UFSACC_READ) {
        if (mode & UFSACC_READ) rc = 1;     /* access allowed */
    }
    if (ufsacc & UFSACC_WRITE) {
        if (mode & UFSACC_WRITE) rc = 1;    /* access allowed */
    }
    if (ufsacc & UFSACC_EXEC) {
        if (mode & UFSACC_EXEC) rc = 1;     /* access allowed */
    }
    /* wtof("%s rc=%u", __func__, rc); */
    return rc;
}
#endif
