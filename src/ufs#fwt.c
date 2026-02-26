#include "ufs.h"
#include "ufs/file.h"

__asm__("\n&FUNC    SETC 'ufs_fwrite'");
UINT32 ufs_fwrite(void *ptr, UINT32 size, UINT32 nitems, UFSFILE *fp)
{
    INT32   rc = 0;
    INT32   lockrc;
    UINT32  items = 0;
    UINT32  n;

    if (!ptr) goto quit;
    if (!size) goto quit;
    if (!nitems) goto quit;
    if (!fp) goto quit;

    /* lock file handle */
    lockrc = lock(fp, 0);

    /* optimize write of simple character array */
    if (size == 1 || nitems == 1) {
        /* calculate number of bytes to read */
        n = size * nitems;

        /* write array of bytes */
        items = ufs_file_putn(fp, ptr, n);

        /* calculate how many items we actually wrote */
        if (items > 0) items = items / size;
        goto done;
    }

    /* write nitems of size size */
    for(; items < nitems; items++) {
        n = ufs_file_putn(fp, ptr, size);
        if (n < size)  break;
        ptr += size;
    }

done:
    /* unlock file handle */
    if (lockrc == 0) unlock(fp, 0);

quit:
    return items;
}
