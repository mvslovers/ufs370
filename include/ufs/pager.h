#ifndef UFS_PAGER_H
#define UFS_PAGER_H

#include <time.h>
#include <stdlib.h>
#include <cliblock.h>           /* lock functions                   */
#include <clibary.h>            /* array functions                  */
#include "ufs/types.h"          /* ufs atomic data types            */
#include "ufs/time.h"           /* time functions                   */
#include "ufs/disk.h"           /* low level disk access prototypes */

typedef struct ufspage      UFSPAGE;    /* block page               */
typedef struct ufspager     UFSPAGER;   /* pager disk cache handle  */
typedef enum   ufspgrlk     UFSPGRLK;   /* pager lock type          */

struct ufspage {
    utime64_t   lru;            /* 00 last recently used time stamp */
    UINT32      dirty;          /* 08 buffer needs to be written    */
                                /*    to disk (deferred write)      */
    UINT32      used;           /* 0C cache used counter            */
    UINT32      block;          /* 10 buffer block number           */
    char        buf[0];         /* 14 buffer contents               */
};

struct ufspager {               /* pager disk cache handle          */
    char        eye[8];         /* 00 eye catcher for dumps         */
#define UFSPAGER_EYE "UFSPAGER" /* ... eye catcher for dumps        */
    UFSDISK     *disk;          /* 08 disk handle                   */
    UINT32      maxpages;       /* 0C maximum number of pages       */
    UFSPAGE     **pages;        /* 10 disk page cache (array)       */
    UINT32      cachehits;      /* 14 number of cache hits          */
    UINT32      reads;          /* 18 number of read request        */
    UINT32      writes;         /* 1C number of write request       */
    UINT32      dreads;         /* 20 number of disk reads          */
    UINT32      dwrites;        /* 24 number of disk writes         */
};                              /* 28 (40 bytes)                    */


/* create new page cache for disk */
UFSPAGER *ufs_pager_new(UFSDISK *disk, UINT32 maxpages)                         asm("UFSPGRND");

/* read block from cache or disk */
INT32 ufs_pager_read(UFSPAGER *pager, void *buf, UINT32 block)                  asm("UFSPGRRD");

/* read one or more blocks from cache or disk */
INT32 ufs_pager_read_n(UFSPAGER *pager, void *buf, UINT32 block, int count)     asm("UFSPGRRN");

/* write block to cache and disk */
INT32 ufs_pager_write(UFSPAGER *pager, void *buf, UINT32 block)                 asm("UFSPGRWT");

/* write one or more blocks to cache and disk */
INT32 ufs_pager_write_n(UFSPAGER *pager, void *buf, UINT32 block, int count)    asm("UFSPGRWN");

/* flush any pending writes to disk */
INT32 ufs_pager_flush(UFSPAGER *pager)                                          asm("UFSPGRFL");

/* free cache for disk, flush deferred write pages */
void ufs_pager_free(UFSPAGER **pager, INT32 flush)                              asm("UFSPGRFR");

/* - - - - Internal pager functions - - - - */

/* ufs_pager_lock() lock type */
enum ufspgrlk {
    UFSPGRLK_TYPE_SHARED=0,
    UFSPGRLK_TYPE_EXCL
};

/* lock pager handle */
INT32 ufs_pager_lock(UFSPAGER *pager, UFSPGRLK type)                            asm("UFSPGRLK");

/* unlock pager handle */
INT32 ufs_pager_unlock(UFSPAGER *pager, UFSPGRLK type)                          asm("UFSPGRUL");

/* lock page */
INT32 ufs_page_lock(UFSPAGE *page, UFSPGRLK type)                               asm("UFSPGLCK");

/* unlock page */
INT32 ufs_page_unlock(UFSPAGE *page, UFSPGRLK type)                             asm("UFSPGUNL");

/* allocate a new cache page */
UFSPAGE *ufs_page_new(unsigned blksize)                                         asm("UFSPGNEW");

/* deallocate a cache page */
void ufs_page_free(UFSPAGE **page)                                              asm("UFSPGFRE");

/* increment a pager counter */
void ufs_pager_inc(UINT32 *counter, UINT32 reset)                               asm("UFSPGRIN");

/* return least recently used page from cache, or new page if below maxpages */
UFSPAGE *ufs_pager_lru_page(UFSPAGER *pager)                                    asm("UFSPGRLP");

/* flush a cache page */
INT32 ufs_page_flush(UFSPAGER *pager, UFSPAGE *page)                            asm("UFSPGFLS");

/* find a block in the cache */
UFSPAGE *ufs_pager_find_block(UFSPAGER *pager, UINT32 block)                    asm("UFSPGRFB");

#endif /* UFS_PAGER_H */
