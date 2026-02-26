#ifndef UFSI_H
#define UFSI_H

/* Borrowed from XINU project with tweaks for MVS38J and CRENT370 */

#include "ufs.h"
#ifndef SECT
#define SECT(name)  asm(name)
#endif

#include <clibstr.h>    /* memcpy(), memset() */
#include <clibwto.h>    /* wto(), wtof(), wtodumpf() */
#include <clibstae.h>   /* try() */
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <time64.h>
#include <osio.h>

#include "ufs/types.h"  /* our atomic data types    */
#include "ufs/disk.h"   /* low level disk i/o       */
#if 0
#include "ufs/pager.h"  /* low level paging/caching */
#endif

#include <kernel.h>
#include <conf.h>
#include <process.h>
#include <queue.h>
#include <sched.h>
#include <semaphore.h>
#include <memory.h>
#include <bufpool.h>
#include <clock.h>
#include <mark.h>
#include <ports.h>
#include <uart.h>
#include <tty.h>
#include <device.h>
#include <interrupt.h>
#include <file.h>
#include <rfilesys.h>
#include <rdisksys.h>
#include <lfilesys.h>
/* #include <ag71xx.h> */
#include <ether.h>
/* #include <mips.h> */
/* #include <nvram.h> */
/* #include <gpio.h> */
#include <net.h>
#include <arp.h>
#include <udp.h>
#include <dhcp.h>
#include <icmp.h>
#include <name.h>
#include <shell.h>
#include <date.h>
#include <prototypes.h>

typedef struct ufsi         UFSI;
#if 0
typedef struct ufsdisk      UFSDISK;
#endif
typedef struct lfdiskinfo   UFSINFO;
typedef struct lflcblk      LFLCBLK;
typedef struct nmentry      NMENTRY;

extern UFSI *ufsiget(void);             /* returns pointer to struct ufs instance */
extern void  ufsiset(struct ufsi*);     /* sets pointer to struct ufs instance    */

/* unix style file system, private data */
struct ufsi {
    char            eye[8];             /* 00 eye catcher for dumps             */
#define UFSI_EYE    "**UFSI**"          /* ... eye catcher for dumps            */
    LFLCBLK         **openfiles;        /* 08 array of open file handles        */
    NMENTRY         **nametab;          /* 0C array of names                    */
    UFSDISK         *disk[Nlfs];        /* 10 array of disk dataset handles (6) */
};                                      /* 28 (40 bytes)                        */

#if 0
/* unix style file system, emulated physical disk (BDAM dataset in MVS) */
struct ufsdisk {                        /* disk dataset handle                  */
    char            eye[8];             /* 00 eye catcher for dumps             */
#define UFSDISK_EYE "UFSDISK"           /* ... eye catcher for dumps            */
    char            ddname[8+1];        /* 08 DD name of disk dataset           */
    byte            flags;              /* 11 processing flags                  */
#define UFSDISK_FLAG_LOAD   0x80        /* ... dataset is open for formatting   */
#define UFSDISK_FLAG_R      0x08        /* ... dataset is open for read access  */
#define UFSDISK_FLAG_W      0x04        /* ... dataset is open for write access */
#define UFSDISK_FLAG_RW     0x0C        /* ... dataset is open for read+write   */
    byte            unused1;            /* 12 unused                            */
    byte            unused2;            /* 13 unused                            */
    DCB             *dcb;               /* 14 DCB for opened disk dataset       */
    UFSINFO         info;               /* 18 disk info (32 bytes)              */
};                                      /* 38 (56 bytes)                        */
#endif /* 0 */


#if 0
    /* XINU extern's */
    qid16           readylist;          /* global ID for list of ready processes*/
    struct procent  proctab[NPROC];
    int32           prcount;            /* currently active processes		*/
    pid32           currpid;            /* currently executing process		*/
    struct qentry   queuetab[NQENT];
    struct defer    Defer;
    struct sentry   semtab[NSEM];
    struct memblk   memlist;            /* head of free memory list	*/
    void            *maxheap;           /* max free memory address	*/
    void            *minheap;           /* address beyond loaded memory	*/
    struct bpentry  buftab[NBPOOLS];    /* Buffer pool table			*/
    bpid32          nbpools;            /* current number of allocated pools	*/
    uint32          clkticks;           /* counts clock interrupts		*/
    uint32          clktime;            /* current time in secs since boot	*/
    qid16           sleepq;             /* queue for sleeping processes		*/
    uint32          preempt;            /* preemption counter			*/
    uint32          *(marks[MM_MAX_MARKS]);
    uint32          marked;
    sid32           mkmutex;
    struct ptnode   *ptfree;            /* list of free nodes		*/
    struct ptentry  porttab[NPORTS];    /* port table			*/
    int32           ptnextid;           /* next port ID to try when	*/
    struct ttycblk  ttytab[Ntty];
    struct rfdata   Rf_data;
    struct rflcblk  rfltab[Nrfl];       /* remote file control blocks	*/
    struct rdscblk  rdstab[Nrds];       /* remote disk control block	*/
    struct lfdata   Lf_data;
    struct lflcblk  lfltab[Nlfl];
    struct ether    ethertab[Neth];     /* array of control blocks */
    struct netpacket *currpkt;          /* ptr to current input packet	*/
    bpid32          netbufpool;         /* ID of net packet buffer pool	*/
    struct network  NetData;            /* Local network interface	*/
    struct arpentry arpcache[ARP_SIZ];
    struct udpentry udptab[UDP_SLOTS];  /* table of UDP endpoints	*/
    uint32          myipaddr;           /* IP address of computer	*/
    struct icmpentry icmptab[ICMP_SLOTS]; /* table of UDP endpoints	*/
    struct nmentry  nametab[NNAMES];    /* table of name mappings	*/
    int32           nnames;             /* num. of entries allocated	*/
    uint32          ncmd;
    const struct cmdent cmdtab[24];
    struct dateinfo Date;               /* Global date information	*/
#endif /* 0 */


#if 0
/* remap the XINU externals to ufs struct items */
#define readylist       ((ufsget())->readylist)
#define proctab         ((ufsget())->proctab)
#define prcount         ((ufsget())->prcount)
#define currpid         ((ufsget())->currpid)
#define queuetab        ((ufeget())->queuetab)
#define Defer           ((ufsget())->Defer)
#define semtab          ((ufsget())->semtab)
#define memlist         ((ufsget())->memlist)
#define maxheap         ((ufsget())->maxheap)
#define minheap         ((ufsget())->minheap)
#define buftab          ((ufsget())->buftab)
#define nbpools         ((ufsget())->nbpools)
#define clkticks        ((ufsget())->clkticks)
#define clktime         ((ufsget())->clktime)
#define sleepq          ((ufsget())->sleepq)
#define preempt         ((ufsget())->preempt)
#define marks           ((ufsget())->marks)
#define marked          ((ufsget())->marked)
#define mkmutex         ((ufsget())->mkmutex)
#define ptfree          ((ufsget())->ptfree)
#define porttab         ((ufsget())->porttab)
#define ptnextid        ((ufsget())->ptnextid)
#define ttytab          ((ufsget())->ttytab)
#define Rf_data         ((ufsget())->Rf_data)
#define rfltab          ((ufsget())->rfltab)
#define rdstab          ((ufsget())->rdstab)
#define Lf_data         ((ufsget())->Lf_data)
#define lfltab          ((ufsget())->lfltab)
#define currpkt         ((ufsget())->currpkt)
#define netbufpool      ((ufsget())->netbufpool)
#define NetData         ((ufsget())->NetData)
#define arpcache        ((ufsget())->arpcache)
#define udptab          ((ufsget())->udptab)
#define currpkt         ((ufsget())->currpkt)
#define netbufpool      ((ufsget())->netbufpool)
#define myipaddr        ((ufsget())->myipaddr)
#define icmptab         ((ufsget())->icmptab)
#define nametab         ((ufsget())->nametab)
#define nnames          ((ufsget())->nnames)
#define ncmd            ((ufsget())->ncmd)
#define cmdtab          ((ufsget())->cmdtab)
#define Date            ((ufsget())->Date)
#endif /* 0 */

#endif /* UFSI_H */
