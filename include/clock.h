#ifndef CLOCK_H
#define CLOCK_H
/* Borrowed from XINU project with tweaks for MVS38J and CRENT370 */

/* clock.h */

#define CLKTICKS_PER_SEC  1000	/* clock timer resolution		*/
#define CLKCYCS_PER_TICK  200000

#if 0
extern	uint32	clkticks;	/* counts clock interrupts		*/
extern	uint32	clktime;	/* current time in secs since boot	*/

extern	qid16	sleepq;		/* queue for sleeping processes		*/
extern	uint32	preempt;	/* preemption counter			*/
#endif /* 0 */

extern	void	clkupdate(uint32)   SECT("CLKUPDAT");

#endif /* CLOCK_H */
