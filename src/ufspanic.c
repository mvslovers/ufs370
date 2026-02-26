#include "ufs/panic.h"

__asm__("\n&FUNC    SETC 'ufs_panic'");
void ufs_panic(const char *fmt, ...)
{
    va_list list;

    /* gripe to the cancole */
    va_start(list, fmt);
    vwtof(fmt, list);
    va_end(list);

    /* call stack trace back to the console */
    wto_traceback(sa_prev(0));
}
