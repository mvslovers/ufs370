#ifndef UFS_NAME_H
#define UFS_NAME_H

#include <time.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <clibary.h>
#include <cliblock.h>
#include "ufs/types.h"
#include "ufs/panic.h"
#include "ufs/vdisk.h"

/* Constants that define the namespace mapping table sizes */
#define	NM_MAXLEN   256             /* maximum size of a file name	*/
#define NM_NAME     "namespace"     /* name of name space device    */

/* Definition of the name prefix table that defines all name mappings */
typedef struct ufs_name     UFSNAME;
struct	ufs_name {                  /* definition of prefix table       */
    char        *prefix;            /* 00 null-terminated prefix	    */
    char        *rep;               /* 04 null-terminated replacement   */
    UFSVDISK    *vdisk;             /* 08 device descriptor for prefix  */
};                                  /* 0C (12 bytes)                    */

INT32 ufs_name_mount(const char *prefix, const char *rep, UFSVDISK *vdisk)  asm("UFSNMMNT");

UFSNAME *ufs_name_find(const char *name)                                    asm("UFSNMFND");
INT32 ufs_name_sort(void)                                                   asm("UFSNMSRT");

INT32 ufs_name_init(UFSDEV *dev)                                            asm("UFSNMINI");
INT32 ufs_name_term(UFSDEV *dev)                                            asm("UFSNMTRM");

#endif /* UFS_NAME_H */
