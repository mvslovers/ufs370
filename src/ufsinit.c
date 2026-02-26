#include <stdlib.h>
#include <string.h>
#include "clibwto.h"
#include "osio.h"
#include "ufsi.h"

int main(int argc, char **argv)
{
    int         rc      = 0;
    int         blksize = 1024;
    char        *buf    = (char*)0;
    DCB         *dcb    = (DCB*)0;
    DECB        decb    = {0};
    float       pctiblk = 10.0;
    UFSINFO     info    = {0};
    unsigned    openpl  = 0x80000000;
    unsigned    block;
    unsigned    blocks;
    int32       i;
    ibid32      next;
    struct lfdbfree *dbfree;
    struct lfiblk *iblk;

    dcb = osbdcb("DISKFILE", NULL);
    if (!dcb) {
        ufs_panic("unable to allocate OUTPUT DCB for DISKFILE");
        goto quit;
    }

    if (argc > 1) {
        blksize = atoi(argv[1]);
    }

    if (blksize == 1024 || blksize == 2048 || blksize == 4096 || blksize == 8192) {
        /* acceptable block sizes */
    }
    else {
        /* gripe and set default block size */
        ufs_panic("Requested block size %d is not allowed.", blksize);
        blksize = 1024;
        ufs_panic("Defaulting to %d block size.", blksize);
    }

    buf = calloc(1, blksize);
    if (!buf) {
        ufs_panic("Unable to allocate %d byte buffer", blksize);
        goto quit;
    }

    memset(buf, 0xFF, blksize);
    dbfree = (struct lfdbfree *)buf;
    iblk   = (struct lfiblk *)buf;

    /* set block size in DCB */
    dcb->dcbblksi = (unsigned short)blksize;

    rc = osbopen(dcb, 0, "load");
    if (rc) {
        ufs_panic("Unable to open DD DISKFILE for OUTPUT");
        goto quit;
    }

    /* write free list blocks to disk dataset. */
    /* we only want to initialize the primary extent, skip secondary extents even if allocated */
    /* rc=4 is end of track, rc=8 is end of extent, rc=12 is write error, no more tracks (SB37) */
    for(i=1; rc<=4; i++) {
        /* point this block to "next" block */
        dbfree->next = i;
        rc = oswrite(&decb, dcb, buf, blksize);

        /* check for real write errors */
        if (rc > 8) ufs_panic("oswrite returned rc=%d for block %d", rc, i);

        /* we have to CHECK to make sure the write completed okay */
        oscheck(&decb);
    }

    ufs_panic("%d blocks initialized", --i);

    memset(&info, 0, sizeof(UFSINFO));
    info.free_iblk      = 0;
    info.total_blocks   = i;

    /* now close the disk dataset */
    osbclose(dcb, NULL, 1, 0);
    dcb = NULL;

    /* open the dataset for BDAM processing */
    dcb = osddcb("DISKFILE");
    if (!dcb) {
        ufs_panic("unable to allocate UPDATE DCB for DISKFILE");
        goto quit;
    }

    rc = osdopen(dcb, 0);
    if (rc) {
        ufs_panic("Unable to open DD DISKFILE for UPDATE, rc=%d", rc);
        goto quit;
    }

    /* format the i-block chain */
    blocks  = (unsigned) (info.total_blocks * (pctiblk / 100.0));
    if (blocks < 2) blocks = 2;
    for(block=ib2sect(0), next = 1; next < blocks; block++) {
        memset(buf, 0xff, blksize);
        iblk = (struct lfiblk *)buf;
        for(i=0; i < LF_IBSBLK; i++) {
            if ((block == ib2sect(0)) && (i==0)) {
                /* reserve this i-block for the file system root */
                iblk->next = LF_INULL;
            }
            else if ((block + 1 == blocks) && (i + 1 == LF_IBSBLK)) {
                /* last i-block in chain of free i-blocks */
                iblk->next = LF_INULL;
            }
            else {
                /* not last i-block in chain of free ib-blocks */
                iblk->next = next;
            }
            next++;
            iblk++;
        }
        rc = osdwrite(&decb, dcb, buf, blksize, block);
        oscheck(&decb);
    }
    info.version     = 1;
    info.blksize     = blksize;
    info.free_dblk   = block;
    info.free_iblk   = 1;    /* first available i-block */
    info.dir_iblk    = 0;    /* file system root i-block */
    info.total_iblks = next;    /* number of i-blocks, not physical blocks */
    info.total_dblks = info.total_blocks - (block-ib2sect(0));  /* number of physical blocks */

    /* update the last free d-block "next" chain */
    block = info.total_blocks - 1;
    rc = osdread(&decb, dcb, buf, blksize, block);
    oscheck(&decb);
    dbfree->next = LF_INULL;
    rc = osdwrite(&decb, dcb, buf, blksize, block);
    oscheck(&decb);

    /* update the file system disk info block */
    block = LF_AREA_DI;   /* disk info area */
    memset(buf, 0xFF, blksize);
    memcpy(buf, &info, sizeof(info));
    rc = osdwrite(&decb, dcb, buf, blksize, block);
    oscheck(&decb);

    osdclose(dcb, 1);
    dcb = NULL;

quit:
    if (buf) free(buf);
    if (dcb) osbclose(dcb, NULL, 1, 0);

    return rc;
}
