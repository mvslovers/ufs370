#ifndef KERNEL_H
#define KERNEL_H

/* Borrowed from XINU project with tweaks for MVS38J and CRENT370 */

/* kernel.h */

/* General type declarations used throughout the kernel */

typedef	unsigned char   byte;
typedef	int             int32;
typedef	short           int16;
typedef	unsigned int    uint32;
typedef	unsigned short  uint16;

/* Xinu-specific types */

typedef	int32           sid32;      /* semaphore ID                         */
typedef	int16           qid16;      /* queue ID                             */
typedef	int32           pid32;      /* process ID                           */
typedef	int32           did32;      /* device ID                            */
typedef	int16           pri16;      /* process priority                     */
typedef	uint32          umsg32;     /* message passed among processes       */
typedef	int32           bpid32;     /* buffer pool ID                       */
typedef	byte            bool8;      /* Boolean type                         */
typedef	uint32          intmask;    /* saved interrupt mask                 */
typedef	int32           ibid32;     /* index block ID (used in file system) */
typedef	int32           dbid32;     /* data block ID (used in file system)  */

/* Function declaration return types */

typedef int32           syscall;    /* system call declaration              */
typedef int32           devcall;    /* device call declaration              */
typedef int32           shellcmd;   /* shell command declaration            */
typedef int32           process;    /* top-level function of a process      */
typedef	void            interrupt;  /* interrupt procedure                  */
typedef	int32           status;     /* returned status value (OK/SYSERR)    */

#define local   static              /* Local procedure or variable declar.	*/

/* Boolean constants */

#define FALSE   0                   /* Boolean False                        */
#define TRUE	1                   /* Boolean True                         */

/* Null pointer, character, and string definintions */

#ifndef NULL
#define NULL	0                   /* null pointer for linked lists        */
#endif
#define NULLCH	'\0'                /* null character                       */
#define	NULLSTR	""                  /* null string                          */

/* Universal return constants */

#define OK      ( 1)                /* normal system call return            */
#define	SYSERR  (-1)                /* system call failed                   */
#define	EOF     (-2)                /* End-of-file (usually from read)      */
#define	TIMEOUT	(-3)                /* system call timed out                */

#if 0   /* let compiler find these external references */
extern	qid16	readylist;	/* global ID for list of ready processes*/
#endif

#define	MINSTK  400                 /* minimum stack size in bytes          */

#define	CONTEXT	64                  /* bytes in a function call context on	*/
                                    /* the run-time stack                   */
#define	QUANTUM	2                   /* time slice in milliseconds           */

/* Size of the stack for the null process */

#define	NULLSTK 8192                /* stack size for null process          */

/* Prototypes of I/O functions used throughout the kernel */

syscall	kprintf(char *fmt, ...)                         SECT("KPRINTF");
#if 0 /* I don't think we'll need these */
syscall	kputc(byte)                                     SECT("KPUTC");
syscall	kgetc(void)                                     SECT("KGETC");
#endif /* 0 */

#endif /* KERNEL_H */
