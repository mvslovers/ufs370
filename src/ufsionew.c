#include "ufs/io.h"

/* create instance of UFSIO handle */
__asm__("\n&FUNC    SETC 'ufs_io_new'");
UFSIO *ufs_io_new(void *ctx, void *readfn, void *read_nfn, void *writefn, void *write_nfn)
{
    UFSIO   *io = calloc(1, sizeof(UFSIO));

    if (io) {
        io->ctx        = ctx;
        io->read_fn    = readfn;
        io->read_n_fn  = read_nfn;
        io->write_fn   = writefn;
        io->write_n_fn = write_nfn;
    }

    return io;
}
