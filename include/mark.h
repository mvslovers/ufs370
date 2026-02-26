#ifndef MARK_H
#define MARK_H
/* Borrowed from XINU project with tweaks for MVS38J and CRENT370 */

/* mark.h - unmarked */

#ifndef	MM_MAX_MARKS
#define	MM_MAX_MARKS	20	/* maximum number of marked locations	*/
#endif

#if 0
extern	uint32	*(marks[]);
extern	uint32	marked;
extern	sid32	mkmutex;
#endif /* 0 */

typedef	uint32	marker[1];	/* Because marker is declared to be an	*/
				/* array, it it unnecessary to use	*/
				/* an ampersand to obtain the address	*/

#define	unmarked(L)		(L[0]<0 || L[0]>=marked || marks[L[0]]!=L)

#endif /* MARK_H */
