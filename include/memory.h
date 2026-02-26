#ifndef MEMORY_H
#define MEMORY_H
/* Borrowed from XINU project with tweaks for MVS38J and CRENT370 */

/* memory.h - roundmb, truncmb, freestk */

#define	PAGE_SIZE	4096
#define MAXADDR		0x02000000	/* 160NL has 32MB RAM		*/

/*----------------------------------------------------------------------
 * roundmb, truncmb - round or truncate address to memory block size
 *----------------------------------------------------------------------
 */
#define	roundmb(x)	(char *)( (7 + (uint32)(x)) & (~7) )
#define	truncmb(x)	(char *)( ((uint32)(x)) & (~7) )

/*----------------------------------------------------------------------
 *  freestk  --  free stack memory allocated by getstk
 *----------------------------------------------------------------------
 */
#define	freestk(p,len)	freemem((char *)((uint32)(p)		\
				- ((uint32)roundmb(len))	\
				+ (uint32)sizeof(uint32)),	\
				(uint32)roundmb(len) )

struct	memblk	{			/* see roundmb & truncmb	*/
	struct	memblk	*mnext;		/* ptr to next free memory blk	*/
	uint32	mlength;		/* size of blk (includes memblk)*/
	};
#if 0
extern	struct	memblk	memlist;	/* head of free memory list	*/
extern	void	*maxheap;		/* max free memory address	*/
extern	void	*minheap;		/* address beyond loaded memory	*/
#endif /* 0 */

/* added by linker */

extern	int	end             SECT("MEMEND");         /* end of program		*/
extern	int	edata           SECT("MEMEDATA");       /* end of data segment		*/
extern	int	etext           SECT("MEMETEXT");       /* end of text segment		*/

#endif /* MEMORY_H */
