#include "ufs.h"
#include "ufs/file.h"

__asm__("\n&FUNC    SETC 'ufs_fgets'");
char *ufs_fgets(char *str, int num, UFSFILE *fp)
{
    INT32   rc = 0;
    INT32   lockrc;
    INT32   i  = 0;
    INT32   c  = 0;

    if (!str) return NULL;
    if (!num) return NULL;
    if (!fp) return NULL;

    /* lock file handle */
    lockrc = lock(fp, 0);

    if (!(fp->mode & FILE_MODE_READ)) {
        /* file handle not for reading, indicate error */
        fp->ind |= FILE_IND_ERROR;
        fp->error = EPERM;
        goto done;
    }

    /* read up to num characters looking for newline */
    while( i < (num-1) ) {
        c = ufs_file_getc(fp);
        if (c < 0) break;   /* EOF or ERROR */

        str[i++] = (char)c;

        if (c=='\n') break; /* EBCDIC newline */
        if (c==0x0A) break; /* ASCII newline */
    }

done:
    /* unlock file handle */
    if (lockrc == 0) unlock(fp, 0);

quit:
    if (i==0) return NULL;  /* nothing read, EOF or ERROR */

    str[i]=0;  /* append zero byte to string */
    return str;
}
