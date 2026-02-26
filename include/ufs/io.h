#ifndef UFS_IO_H
#define UFS_IO_H

#include <stdlib.h>
#include <string.h>
#include "ufs/types.h"
#include "ufs/panic.h"

struct ufsio {
    /* context (user data) passed to these function pointers */
    void *ctx;

    /* read block from cache or disk */
    INT32 (*read_fn)(void *ctx, void *buf, UINT32 block);

    /* read one or more blocks from cache or disk */
    INT32 (*read_n_fn)(void *ctx, void *buf, UINT32 block, int count);

    /* write block to cache and disk */
    INT32 (*write_fn)(void *ctx, void *buf, UINT32 block);

    /* write one or more blocks to cache and disk */
    INT32 (*write_n_fn)(void *ctx, void *buf, UINT32 block, int count);
};

/* create instance of UFSIO handle */
UFSIO *ufs_io_new(void *ctx, void *readfn, void *read_nfn, void *writefn, void *write_nfn)  asm("UFSIONEW");

/* free instance of UFSIO handle */
void ufs_io_free(UFSIO **ufsio)                                                             asm("UFSIOFRE");

#endif /* UFS_IO_H */
