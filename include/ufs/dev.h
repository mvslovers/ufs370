#ifndef UFS_DEV_H
#define UFS_DEV_H

#include <time.h>
#include <stdlib.h>
#include <errno.h>
#include <clibary.h>
#include <cliblock.h>
#include "ufs/types.h"
#include "ufs/panic.h"

struct ufs_devio {
    INT32       (*dvinit) (UFSDEV *);                       /* 00 optional  */
    INT32       (*dvopen) (UFSDEV *, char *, char *);       /* 04           */
    INT32       (*dvclose)(UFSDEV *);                       /* 08           */
    INT32       (*dvread) (UFSDEV *, void *, UINT32);       /* 0C           */
    INT32       (*dvwrite)(UFSDEV *, void *, UINT32);       /* 10           */
    INT32       (*dvseek) (UFSDEV *, INT32);                /* 14           */
    INT32       (*dvgetc) (UFSDEV *);                       /* 18           */
    INT32       (*dvputc) (UFSDEV *, INT32);                /* 1C           */
    INT32       (*dvcntl) (UFSDEV *, INT32, INT32, INT32);  /* 20           */
    INT32       (*dvterm) (UFSDEV *);                       /* 24 optional  */
};                                      /* 28 (40 bytes)                    */

extern UFSDEVIO ufsdevio;

struct ufs_dev {
    char        eye[8];                 /* 00 eye catcher for dumps         */
#define UFSDEV_EYE  "*UFSDEV*"          /* ... eye catcher for dumps        */
    INT32       num;                    /* 08 device number                 */
    INT32       minor;                  /* 0C device minor number           */
#define UFSDEV_MAX_NAME     15          /* Max device name length           */
    char        name[UFSDEV_MAX_NAME+1];/* 10 device assigned name          */
    void        *sys;                   /* 20 system handle                 */
    UFSDEV      *parent;                /* 24 parent device handle          */
    void        *ctx1;                  /* 28 device context private data#1 */
    void        *ctx2;                  /* 2C device context private data#2 */
    UFSDEV      **children;             /* 30 devices created by this device*/
    UFSDEVIO    io;                     /* 34 device I/O functions          */
};                                      /* 58 (88 bytes)                    */


/* device instance */
UFSDEV *ufs_dev_new(INT32 num, INT32 minor, const char *name,
                    void *sys, UFSDEV *parent,
                    void *ctx1, void *ctx2, const UFSDEVIO *devio)              asm("UFSDVNEW");
void ufs_dev_free(UFSDEV **dev)                                                 asm("UFSDVFRE");
UFSDEV *ufs_dev_find(const char *name)                                          asm("UFSDVFND");
UFSDEV *ufs_dev_find_num(UINT32 num)                                            asm("UFSDVFNZ");

/* device I/O function wrappers */
INT32 ufs_dev_init(UFSDEV *dev)                                                 asm("UFSDVINI");
INT32 ufs_dev_open(UFSDEV *dev, char *filename, char *mode)                     asm("UFSDVOPN");
INT32 ufs_dev_close(UFSDEV *dev)                                                asm("UFSDVCLS");
INT32 ufs_dev_read(UFSDEV *dev, void *buf, UINT32 sz)                           asm("UFSDVRD");
INT32 ufs_dev_write(UFSDEV *dev, void *buf, UINT32 sz)                          asm("UFSDVWT");
INT32 ufs_dev_seek(UFSDEV *dev, INT32 whence)                                   asm("UFSDVSEE");
INT32 ufs_dev_getc(UFSDEV *dev)                                                 asm("UFSDVGTC");
INT32 ufs_dev_putc(UFSDEV *dev, INT32 c)                                        asm("UFSDVPTC");
INT32 ufs_dev_cntl(UFSDEV *dev, INT32 a, INT32 b, INT32 c)                      asm("UFSDVCTL");
INT32 ufs_dev_term(UFSDEV *dev)                                                 asm("UFSDVTRM");

/* dummy functions */
INT32 ufs_dev_null(UFSDEV *dev)      /* always returns 0 */                     asm("UFSDVNUL");
INT32 ufs_dev_ioerr(UFSDEV *dev)     /* always returns EIO */                   asm("UFSDVERR");

#endif /* UFS_DEV_H */
