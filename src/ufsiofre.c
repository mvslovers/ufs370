#include "ufs/io.h"

/* create instance of UFSIO handle */
__asm__("\n&FUNC    SETC 'ufs_io_free'");
void ufs_io_free(UFSIO **ufsio)
{
    if (ufsio && *ufsio) {
        free(*ufsio);
        *ufsio = NULL;
    }
}
