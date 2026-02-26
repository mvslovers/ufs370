#include "ufs.h"
#include "ufs/file.h"

__asm__("\n&FUNC    SETC 'ufs_fclose'");
void ufs_fclose(UFSFILE **file)
{
    UFSFILE     *fp         = NULL;

    if (!file) return;
    fp = *file;

    if (!fp) return;

    /* sync file handle to disk */
    ufs_fsync(fp);

    /* free the file handle */
    ufs_file_free(file);
}
