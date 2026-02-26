#include "ufs/file.h"

__asm__("\n&FUNC    SETC 'ufs_file_mode'");
BYTE ufs_file_mode(const char *mode)
{
    int     i;
    UINT32  flags   = 0;

    if (!mode) goto quit;

    for (i=0; mode[i]; i++) {
        char    c = mode[i];

        /* skip any invalid characters in mode string */
        if (!strchr("abrw+", c)) continue;

        /* the first character of the mode string must be 'r', 'w' or 'a' */
        if (i==0) {
            /* first character sets basic mode: read, write, append */
            if (c=='r') {
                flags |= FILE_MODE_READ;
            }
            else if (c=='w') {
                flags |= FILE_MODE_WRITE + FILE_MODE_CREATE + FILE_MODE_TRUNC;
            }
            else if (c=='a') {
                flags |= FILE_MODE_WRITE + FILE_MODE_CREATE + FILE_MODE_APPEND;
            }
            continue;
        }

        /* only 'b' or '+' are allowed as modifiers */
        if (c=='b') {
            /* "rb" */
            flags |= FILE_MODE_BINARY;
            continue;
        }
        if (c=='+') {
            if (flags & FILE_MODE_READ) {
                /* "r+", set 'w' */
                flags |= FILE_MODE_WRITE;
            }
            else if (flags & FILE_MODE_WRITE) {
                /* "w+", set 'r' */
                flags |= FILE_MODE_READ;
            }
            continue;
        }

        /* non-standard modifiers, allow "?w" or "?r" instead of "?+" mode flags */
        if (c=='r') {
            flags |= FILE_MODE_READ;
            continue;
        }
        if (c=='w') {
            flags |= FILE_MODE_WRITE;
            continue;
        }
    }

quit:
    return (BYTE) flags;
}
