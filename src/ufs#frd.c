#include "ufs.h"
#include "ufs/file.h"

__asm__("\n&FUNC    SETC 'ufs_fread'");
UINT32 ufs_fread(void *ptr, UINT32 size, UINT32 nitems, UFSFILE *fp)
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

    /* optimize read of simple character array */
    if (size == 1 || nitems == 1) {
        /* calculate number of bytes to read */
        n = size * nitems;

        /* read array of bytes */
        items = ufs_file_getn(fp, ptr, n);

        /* calculate how many items we actually read */
        if (items > 0) items = items / size;
        goto done;
    }

    /* read nitems of size size */
    for(; items < nitems; items++) {
        n = ufs_file_getn(fp, ptr, size);
        if (n < size)  break;
        ptr += size;
    }

done:
    /* unlock file handle */
    if (lockrc == 0) unlock(fp, 0);

quit:
    return items;
}
