#include "ufsi.h"

extern	devcall	ioerr(void);
extern	devcall	ionull(void);

/* Device independent I/O switch */

struct	dentry	devtab[NDEVS] =
{
/**
 * Format of entries is:
 * dev-number, minor-number, dev-name,
 *   init, open, close,
 *   read, write, seek,
 *   getc, putc, control,
 *   dev-csr-address, intr-handler, irq
 */
/* LFILESYS is lfs (file system root device) */
	{ 0, 0, "LFILESYS",
	  (void *)lfsInit, (void *)lfsOpen, (void *)ioerr,
	  (void *)ioerr, (void *)ioerr, (void *)ioerr,
	  (void *)ioerr, (void *)ioerr, (void *)ioerr,
	  (void *)0x0, (void *)ionull, 0 },

/* LFILESYS1 is lfs */
	{ 1, 0, "LFILESYS1",
	  (void *)lfsInit, (void *)lfsOpen, (void *)ioerr,
	  (void *)ioerr, (void *)ioerr, (void *)ioerr,
	  (void *)ioerr, (void *)ioerr, (void *)ioerr,
	  (void *)0x0, (void *)ionull, 0 },

/* LFILESYS2 is lfs */
	{ 2, 0, "LFILESYS2",
	  (void *)lfsInit, (void *)lfsOpen, (void *)ioerr,
	  (void *)ioerr, (void *)ioerr, (void *)ioerr,
	  (void *)ioerr, (void *)ioerr, (void *)ioerr,
	  (void *)0x0, (void *)ionull, 0 },

/* LFILESYS3 is lfs */
	{ 3, 0, "LFILESYS3",
	  (void *)lfsInit, (void *)lfsOpen, (void *)ioerr,
	  (void *)ioerr, (void *)ioerr, (void *)ioerr,
	  (void *)ioerr, (void *)ioerr, (void *)ioerr,
	  (void *)0x0, (void *)ionull, 0 },

/* LFILESYS4 is lfs */
	{ 4, 0, "LFILESYS4",
	  (void *)lfsInit, (void *)lfsOpen, (void *)ioerr,
	  (void *)ioerr, (void *)ioerr, (void *)ioerr,
	  (void *)ioerr, (void *)ioerr, (void *)ioerr,
	  (void *)0x0, (void *)ionull, 0 },

/* LFILESYS5 is lfs */
	{ 5, 0, "LFILESYS5",
	  (void *)lfsInit, (void *)lfsOpen, (void *)ioerr,
	  (void *)ioerr, (void *)ioerr, (void *)ioerr,
	  (void *)ioerr, (void *)ioerr, (void *)ioerr,
	  (void *)0x0, (void *)ionull, 0 },

/* LFILE is lfl (files opened by lfsOpen() are assigned this device) */
	{ 6, 0, "LFILE",
	  (void *)lflInit, (void *)ioerr, (void *)lflClose,
	  (void *)lflRead, (void *)lflWrite, (void *)lflSeek,
	  (void *)lflGetc, (void *)lflPutc, (void *)lflControl,
	  (void *)0x0, (void *)ionull, 0 },

/* TESTDISK is ram */
	{ 7, 0, "TESTDISK",
	  (void *)ramInit, (void *)ramOpen, (void *)ramClose,
	  (void *)ramRead, (void *)ramWrite, (void *)ioerr,
	  (void *)ioerr, (void *)ioerr, (void *)ioerr,
	  (void *)0x0, (void *)ionull, 0 },

/* NAMESPACE is nam */
	{ 8, 0, "NAMESPACE",
	  (void *)namInit, (void *)namOpen, (void *)ioerr,
	  (void *)ioerr, (void *)ioerr, (void *)ioerr,
	  (void *)ioerr, (void *)ioerr, (void *)ioerr,
	  (void *)0x0, (void *)ioerr, 0 },

/* NULLDEV is null */
	{ 9, 0, "NULLDEV",
	  (void *)ionull, (void *)ionull, (void *)ionull,
	  (void *)ionull, (void *)ionull, (void *)ioerr,
	  (void *)ionull, (void *)ionull, (void *)ioerr,
	  (void *)0x0, (void *)ioerr, 0 },

/* DFILE is dfl (directories opened by lfsOpen() are assigned this device) */
    { 10, 0, "LDIR",
	  (void *)dflInit, (void *)ioerr, (void *)dflClose,
	  (void *)dflRead, (void *)dflWrite, (void *)dflSeek,
	  (void *)ioerr, (void *)ioerr, (void *)dflControl,
	  (void *)0x0, (void *)ionull, 0 },
};
