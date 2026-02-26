#ifndef UFS_PANIC_H
#define UFS_PANIC_H

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <clibwto.h>

void ufs_panic(const char *fmt, ...)            asm("UFSPANIC");

#endif /* UFS_PANIC_H */
