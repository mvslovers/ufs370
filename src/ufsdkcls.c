#include "ufs/disk.h"

/* close disk dataset - deallocates UFSDISK handle */
__asm__("\n&FUNC    SETC 'ufs_disk_close'");
void ufs_disk_close(UFSDISK **disk)
{
    UFSDISK     *d;
    UINT32      count, n;

    if (disk && *disk) {
        d = *disk;

        if (d->dcb) {
            /* we have a DCB handle */
            if (d->dcb->dcboflg & DCBOFOPN) {
                /* if the super block was modified */
                if (d->sb.modified) {
                    int rc = ufs_disk_write_super(d, &d->sb);
                    if (rc) ufs_panic(UFSDK011E, rc);
                }
                /* close the dataset for the DCB */
                osdclose(d->dcb, 0);
            }
            /* release the DCB storage */
            free(d->dcb);
            d->dcb = NULL;
        }

        /* free disk buffers */
        if (d->buf) {
            count = array_count(&d->buf);

            for(n=0; n < count; n++) {
                if (d->buf[n]) free(d->buf[n]);
            }

            array_free(&d->buf);
            d->buf = NULL;
        }

#if 1
        /* release boot extension */
        if (d->boote) {
            free(d->boote);
            d->boote = NULL;
        }
#endif

        /* release the disk handle */
        free(d);
        *disk = NULL;
    }
}
