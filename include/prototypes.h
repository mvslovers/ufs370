#ifndef PROTOTYPES_H
#define PROTOTYPES_H
/* Borrowed from XINU project with tweaks for MVS38J and CRENT370 */

/* in file addargs.c */
extern	status	addargs(pid32, int32, int32[], int32,char *, void *)    SECT("XADDARGS");

/* in file arp.c */
extern	int32	arp_alloc(void)                                         SECT("XARPALLC");
extern	void	arp_in(void)                                            SECT("XARPIN");
extern	void	arp_init(void)                                          SECT("XARPINIT");
extern	status	arp_resolve(uint32, byte *)                             SECT("XARPRESL");

/* in file ascdate.c */
extern	status	ascdate(uint32, char *)                                 SECT("XASCDATE");

/* in file bufinit.c */
extern	status	bufinit(void)                                           SECT("XBUFINIT");

/* in file chprio.c */
extern	pri16	chprio(pid32, pri16)                                    SECT("XCHPRIO");

/* in file clkupdate.S */
extern	uint32	clkcount(void)                                          SECT("XCLKCNT");

/* in file clkhandler.c */
extern	interrupt clkhandler(void)                                      SECT("XCLKHNDL");

/* in file clkinit.c */
extern	void	clkinit(void)                                           SECT("XCLKINIT");

/* in file close.c */
extern	syscall	close(did32)                                            SECT("XCLOSE");

/* in file control.c */
extern	syscall	control(did32, int32, int32, int32)                     SECT("XCONTROL");

/* in file create.c */
extern	pid32	create(void *, uint32, pri16, char *, uint32, ...)      SECT("XCREATE");

/* in file ctxsw.S */
extern	void	ctxsw(void *, void *)                                   SECT("XCTXSW");

/* in file dot2ip.c */
extern	uint32	dot2ip(char *, uint32 *)                                SECT("XDOT2IP");

/* in file queue.c */
extern	pid32	enqueue(pid32, qid16)                                   SECT("XENQUE");

/* in file intutils.S */
extern	intmask	disable(void)                                           SECT("XDISABLE");

/* in file intutils.S */
extern	void	enable(void)                                            SECT("XENABLE");

/* in file ethClose.c */
extern	devcall	ethClose(struct dentry *)                               SECT("XETHCLS");

/* in file ethControl.c */
extern	devcall	ethControl(struct dentry *, int32, int32, int32)        SECT("XETHCTL");

/* in file ethInit.c */
extern	devcall	ethInit(struct dentry *)                                SECT("XETHINIT");

/* in file etherInterupt.c */
extern	interrupt	ethInterrupt(void)                                  SECT("XETHINTR");

/* in file ethOpen.c */
extern	devcall	ethOpen(struct dentry *)                                SECT("XETHOPEN");

/* in file ethRead.c */
extern	devcall ethRead(struct dentry *, void *, uint32)                SECT("XETHREAD");

/* in file ethStat.c */
extern	void	ethStat(uint16)                                         SECT("XETHSTAT");

/* in file ethWrite.c */
extern	devcall	ethWrite(struct dentry *, void *, uint32)               SECT("XETHWRIT");

/* in file exception.c */
extern  void exception(int32, int32*)                                   SECT("XEXCPTN");

/* in file freebuf.c */
extern	syscall	freebuf(char *)                                         SECT("XFREEBUF");

/* in file freemem.c */
extern	syscall	freemem(char *, uint32)                                 SECT("XFREEMEM");

/* in file getbuf.c */
extern	char	*getbuf(bpid32)                                         SECT("XGETBUF");

/* in file getc.c */
extern	syscall	getc(did32)                                             SECT("XGETC");

/* in file getitem.c */
extern	pid32	getfirst(qid16)                                         SECT("XGETFRST");

/* in file dhcp.c */
extern	uint32	getlocalip(void)                                        SECT("XGETLCIP");

/* in file getmem.c */
extern	char	*getmem(uint32)                                         SECT("XGETMEM");

/* in file getpid.c */
extern	pid32	getpid(void)                                            SECT("XGETPID");

/* in file getprio.c */
extern	syscall	getprio(pid32)                                          SECT("XGETPRIO");

/* in file getstk.c */
extern	char	*getstk(uint32)                                         SECT("XGETSTK");

/* in file gettime.c */
extern	status	gettime(uint32 *)                                       SECT("XGETTIME");

/* in file getutime.c */
extern	status	getutime(uint32 *)                                      SECT("XGETUTIM");

/* in file halt.S */
extern	void	halt(void)                                              SECT("XHALT");

/* in file icmp.c */
extern	void	icmp_init (void)                                        SECT("XICMPINI");
extern	void	icmp_in (void)                                          SECT("XICMPIN");
extern	process	icmp_out (void)                                         SECT("XICMPOUT");
extern	int32	icmp_register (uint32)                                  SECT("XICMPREG");
extern	int32	icmp_recv (int32, char *, int32, uint32)                SECT("XICMPREC");
extern	status	icmp_send (uint32, uint16, uint16, uint16, char	*, int32) SECT("XICMPSND");
extern	status	icmp_release (int32)                                    SECT("XICMPREL");
extern	uint16	icmp_cksum (char *, int32)                              SECT("XICMPSUM");

/* in file init.c */
extern	syscall	init(did32)                                             SECT("XINIT");

/* in file insert.c */
extern	status	insert(pid32, qid16, int32)                             SECT("XINSERT");

/* in file insertd.c */
extern	status	insertd(pid32, qid16, int32)                            SECT("XINSERTD");

/* in file ioerr.c */
extern	devcall	ioerr(void)                                             SECT("XIOERR");

/* in file ionull.c */
extern	devcall	ionull(void)                                            SECT("XIONULL");

/* in file ipdump.c */
extern	void	ipdump(char *, int32)                                   SECT("XIPDUMP");

/* in file netin.c */
extern	uint16	ipcksum(struct netpacket *)                             SECT("XIPCHKSM");

/* in file kill.c */
extern	syscall	kill(pid32)                                             SECT("XKILL");

/* in file lexan.c */
extern	int32	lexan(char *, int32, char *, int32 *, int32 [], int32 []) SECT("XLEXAN");

/* in file lfibclear.c */
extern	void	lfibclear(struct lfiblk *, int32)                       SECT("XLFIBCLR");

/* in file lfibget.c */
extern	void	lfibget(did32, ibid32, struct lfiblk *)                 SECT("XLFIBGET");

/* in file lfibput.c */
extern	status	lfibput(did32, ibid32, struct lfiblk *)                 SECT("XLFIBPUT");

/* in file lfdbfree.c */
extern	status	lfdbfree(did32, dbid32)                                 SECT("XLFDBFRE");

/* in file lfdballoc.c */
extern	dbid32	lfdballoc(struct lfdbfree *)                            SECT("XLFDBALC");

/* in file lfflush.c */
extern	status	lfflush(struct lflcblk *)                               SECT("XLFFLUSH");

/* in file lfgetmode.c */
extern	int32	lfgetmode(char *)                                       SECT("XLFGETMO");

/* in file lfiballoc.c */
extern	ibid32	lfiballoc(void)                                         SECT("XLFIBALC");

/* in file lflClose.c */
extern	devcall	lflClose(struct dentry *)                               SECT("XLFLCLS");

/* in file lflControl.c */
extern	devcall	lflControl(struct dentry *, int32, int32, int32)        SECT("XLFLCNTL");

/* in file lflGetc.c */
extern	devcall	lflGetc(struct dentry *)                                SECT("XLFLGETC");

/* in file lflInit.c */
extern	devcall	lflInit(struct dentry *)                                SECT("XLFLINIT");

/* in file lflPutc.c */
extern	devcall	lflPutc(struct dentry *, char)                          SECT("XLFLPUTC");

/* in file lflRead.c */
extern	devcall	lflRead(struct dentry *, char *, int32)                 SECT("XLFLREAD");

/* in file lflSeek.c */
extern	devcall	lflSeek(struct dentry *, uint32)                        SECT("XLFLSEEK");

/* in file lflWrite.c */
extern	devcall	lflWrite(struct dentry *, char *, int32)                SECT("XLFLWRIT");

/* in file lfscreate.c */
extern  status  lfscreate(did32, ibid32, uint32)                        SECT("XLFSCRTE");

/* in file lfsInit.c */
extern	devcall	lfsInit(struct dentry *)                                SECT("XLFSINIT");

/* in file lfsOpen.c */
extern	devcall	lfsOpen(struct dentry *, char *, char *)                SECT("XLFSOPEN");

/* in file lfsetup.c */
extern	status	lfsetup(struct lflcblk *)                               SECT("XLFSETUP");

/* in file lftruncate.c */
extern	status	lftruncate(struct lflcblk *)                            SECT("XFTRUNC");

/* in file lpgetc.c */
extern	devcall	lpgetc(struct dentry *)                                 SECT("XLPGETC");

/* in file lpinit.c */
extern	devcall	lpinit(struct dentry *)                                 SECT("XLPINIT");

/* in file lpopen.c */
extern	devcall	lpopen(struct dentry *, char *, char *)                 SECT("XLPOPEN");

/* in file lpputc.c */
extern	devcall	lpputc(struct dentry *, char)                           SECT("XLPPUTC");

/* in file lpread.c */
extern	devcall	lpread(struct dentry *, char *, int32)                  SECT("XLPREAD");

/* in file lpwrite.c */
extern	devcall	lpwrite(struct dentry *, char *, int32)                 SECT("XLPWRITE");

/* in file mark.c */
extern	void	_mkinit(void)                                           SECT("XMKINIT");

#if 0   /* use clib memset and memcpy */
/* in file memcpy.c */
extern	void	*memcpy(void *, const void *, int32)                    SECT("XMEMCPY");

/* in file memset.c */
extern  void    *memset(void *, const int, int32)                       SECT("XMEMSET");
#endif /* 0 */

/* in file mkbufpool.c */
extern	bpid32	mkbufpool(int32, int32)                                 SECT("XMKBUFPL");

/* in file mount.c */
extern	syscall	mount(char *, char *, did32)                            SECT("XMOUNT");
extern	int32	namlen(char *, int32)                                   SECT("XNAMLEN");

/* in file namInit.c */
extern	status	namInit(void)                                           SECT("XNAMINIT");

/* in file nammap.c */
extern	devcall	nammap(char *, char[], did32)                           SECT("XNAMMAP");
extern	did32	namrepl(char *, char[])                                 SECT("XNAMREPL");
extern	status	namcpy(char *, char *, int32)                           SECT("XNAMCPY");

/* in file namOpen.c */
extern	devcall	namOpen(struct dentry *, char *, char *)                SECT("XNAMOPEN");

/* in file netin.c */
extern	process	netin(void)                                             SECT("XNETIN");

/* in file newqueue.c */
extern	qid16	newqueue(void)                                          SECT("XNEWQUE");

/* in file open.c */
extern	syscall	open(did32, char *, char *)                             SECT("XOPEN");

/* in file panic.c */
extern	void	panic(char *)                                           SECT("XPANIC");

/* in file pdump.c */
extern	void	pdump(struct netpacket *)                               SECT("XPDUMP");

/* in file ptclear.c */
extern	void	_ptclear(struct ptentry *, uint16, int32 (*)(int32))    SECT("XPTCLEAR");

/* in file ptcount.c */
extern	int32	ptcount(int32)                                          SECT("XPTCOUNT");

/* in file ptcreate.c */
extern	syscall	ptcreate(int32)                                         SECT("XPTCREAT");

/* in file ptdelete.c */
extern	syscall	ptdelete(int32, int32 (*)(int32))                       SECT("XPTDELET");

/* in file ptinit.c */
extern	syscall	ptinit(int32)                                           SECT("XPTINIT");

/* in file ptrecv.c */
extern	uint32	ptrecv(int32)                                           SECT("XPTRECV");

/* in file ptreset.c */
extern	syscall	ptreset(int32, int32 (*)(int32))                        SECT("XPTRESET");

/* in file ptsend.c */
extern	syscall	ptsend(int32, umsg32)                                   SECT("XPTSEND");

/* in file putc.c */
extern	syscall	putc(did32, char)                                       SECT("XPUTC");

/* in file ramClose.c */
extern	devcall	ramClose(struct dentry *)                               SECT("XRAMCLS");

/* in file ramInit.c */
extern	devcall	ramInit(struct dentry *)                                SECT("XRAMINIT");

/* in file ramOpen.c */

extern	devcall	ramOpen(struct dentry *, char *, char *)                SECT("XRAMOPEN");

/* in file ramRead.c */
extern	devcall	ramRead(struct dentry *, char *, int32)                 SECT("XRAMREAD");

/* in file ramWrite.c */
extern	devcall	ramWrite(struct dentry *, char *, int32)                SECT("XRAMWRIT");

/* in file rdsClose.c */
extern	devcall	rdsClose(struct dentry *)                               SECT("XRDSCLS");

/* in file rdsControl.c */
extern	devcall	rdsControl(struct dentry *, int32, int32, int32)        SECT("XRDSCNTL");

/* in file rdsInit.c */
extern	devcall	rdsInit(struct dentry *)                                SECT("XRDSINIT");

/* in file rdsOpen.c */
extern	devcall	rdsOpen(struct dentry *, char *, char *)                SECT("XRDSOPEN");

/* in file rdsRead.c */
extern	devcall	rdsRead(struct dentry *, char *, int32)                 SECT("XRDSREAD");

/* in file rdsWrite.c */
extern	devcall	rdsWrite(struct dentry *, char *, int32)                SECT("XRDSWRIT");

/* in file rdsbufalloc.c */
extern	struct	rdbuff * rdsbufalloc(struct rdscblk *)                  SECT("XRDSBUFA");

/* in file rdscomm.c */
extern	status	rdscomm(struct rd_msg_hdr *, int32, struct rd_msg_hdr *,
		int32, struct rdscblk *)                                        SECT("XRDSCOMM");

/* in file rdsprocess.c */
extern	void	rdsprocess(struct rdscblk *)                            SECT("XRDSPROC");

/* in file read.c */
extern	syscall	read(did32, char *, uint32)                             SECT("XREAD");

/* in file ready.c */
extern	status	ready(pid32, bool8)                                     SECT("XREADY");

/* in file receive.c */
extern	umsg32	receive(void)                                           SECT("XRECEIVE");

/* in file recvclr.c */
extern	umsg32	recvclr(void)                                           SECT("XRECVCLR");

/* in file recvtime.c */
extern	umsg32	recvtime(int32)                                         SECT("XRECVTIM");

/* in file resched.c */
extern	void	resched(void)                                           SECT("XRESCHED");

/* in file intutils.S */
extern	void	restore(intmask)                                        SECT("XRESTORE");

/* in file resume.c */
extern	pri16	resume(pid32)                                           SECT("XRESUME");

/* in file rfsgetmode.c */
extern	int32	rfsgetmode(char * )                                     SECT("XRFSGMOD");

/* in file rflClose.c */
extern	devcall	rflClose(struct dentry *)                               SECT("XRFLCLS");

/* in file rfsControl.c */
extern	devcall	rfsControl(struct dentry *, int32, int32, int32)        SECT("XRFSCNTL");

/* in file rflGetc.c */
extern	devcall	rflGetc(struct dentry *)                                SECT("XRFLGETC");

/* in file rflInit.c */
extern	devcall	rflInit(struct dentry *)                                SECT("XRFLINIT");

/* in file rflPutc.c */
extern	devcall	rflPutc(struct dentry *, char )                         SECT("XRFLPUTC");

/* in file rflRead.c */
extern	devcall	rflRead(struct dentry *, char *, int32 )                SECT("XRFLREAD");

/* in file rflSeek.c */
extern	devcall	rflSeek(struct dentry *, uint32 )                       SECT("XRFLSEEK");

/* in file rflWrite.c */
extern	devcall	rflWrite(struct dentry *, char *, int32 )               SECT("XRFLWRIT");

/* in file rfsndmsg.c */
extern	status	rfsndmsg(uint16, char *)                                SECT("XRFSSMSG");

/* in file rfsInit.c */
extern	devcall	rfsInit(struct dentry *)                                SECT("XRFSINIT");

/* in file rfsOpen.c */
extern	devcall	rfsOpen(struct dentry  *devptr, char *, char *)         SECT("XRFSOPEN");

/* in file rfscomm.c */
extern	int32	rfscomm(struct rf_msg_hdr *, int32, struct rf_msg_hdr *, int32) SECT("XRFSCOMM");

/* in file rdsClose.c */
extern	devcall	rdsClose(struct dentry *)                               SECT("XRDSCLS");

/* in file rdsControl.c */
extern	devcall	rdsControl(struct dentry *, int32, int32, int32)        SECT("XRDSCNTL");

/* in file rdsInit.c */
extern	devcall	rdsInit(struct dentry *)                                SECT("XRDSINIT");

/* in file rdsOpen.c */
extern	devcall	rdsOpen(struct dentry *, char *, char *)                SECT("XRDSOPEN");

/* in file rdsRead.c */
extern	devcall	rdsRead(struct dentry *, char *, int32)                 SECT("XRDSREAD");

/* in file rdsWrite.c */
extern	devcall	rdsWrite(struct dentry *, char *, int32)                SECT("XRDSWRIT");

/* in file rdsbufalloc.c */
extern	struct	rdbuff * rdsbufalloc(struct rdscblk *)                  SECT("XRDSBUFA");

/* in file rdscomm.c */
extern	status	rdscomm(struct rd_msg_hdr *, int32, struct rd_msg_hdr *, int32, struct rdscblk *) SECT("XRDSCOMM");

/* in file rdsprocess.c */
extern	void	rdsprocess(struct rdscblk *)                            SECT("XRDSPROC");

/* in file sched_cntl.c */
extern	status	sched_cntl(int32)                                       SECT("XSCHDCTL");

/* in file seek.c */
extern	syscall	seek(did32, uint32)                                     SECT("XSEEK");

/* in file semcount.c */
extern	syscall	semcount(sid32)                                         SECT("XSEMCNT");

/* in file semcreate.c */
extern	sid32	semcreate(int32)                                        SECT("XSEMCRTE");

/* in file semdelete.c */
extern	syscall	semdelete(sid32)                                        SECT("XSEMDLTE");

/* in file semreset.c */
extern	syscall	semreset(sid32, int32)                                  SECT("XSEMRST");

#if 0 /* send() defintion conflicts with send() in socket.h */
/* in file send.c */
extern	syscall	send(pid32, umsg32)                                     SECT("XSEND");
#endif

#if 0 /* conflicts with signal in /home/projects/crent370/include/signal.h */
/* in file signal.c */
extern	syscall	signal(sid32)                                           SECT("XSIGNAL");
#endif

/* in file signaln.c */
extern	syscall	signaln(sid32, int32)                                   SECT("XSIGNALN");

/* in file sleep.c */
extern	syscall	sleepms(uint32)                                         SECT("XSLEEPMS");
extern	syscall	sleep(uint32)                                           SECT("XSLEEP");

/* in file suspend.c */
extern	syscall	suspend(pid32)                                          SECT("XSUSPEND");

/* in file ttyControl.c */
extern	devcall	ttyControl(struct dentry *, int32, int32, int32)        SECT("XTTYCTL");

/* in file ttyGetc.c */
extern	devcall	ttyGetc(struct dentry *)                                SECT("XTTYGETC");

/* in file ttyInter_in.c */
extern	void	ttyInter_in(struct ttycblk *, struct uart_csreg *)      SECT("XTTYINTI");

/* in file ttyInter_out.c */
extern	void	ttyInter_out(struct ttycblk *, struct uart_csreg *)     SECT("XTTYINIO");

/* in file ttyInterrupt.c */
extern	interrupt	ttyInterrupt(void)                                  SECT("XTTYINTR");

/* in file ttyInit.c */
extern	devcall	ttyInit(struct dentry *)                                SECT("XTTYINIT");

/* in file ttyKickOut.c */
extern	void	ttyKickOut(struct ttycblk *, struct uart_csreg *)       SECT("XTTYKOUT");

/* in file ttyPutc.c */
extern	devcall	ttyPutc(struct dentry *, char)                          SECT("XTTYPUTC");

/* in file ttyRead.c */
extern	devcall	ttyRead(struct dentry *, char *, int32)                 SECT("XTTYREAD");

/* in file ttyWrite.c */
extern	devcall	ttyWrite(struct dentry *, char *, int32)                SECT("XTTYWRIT");

/* in file udp.c */
extern	void	udp_in(void)                                            SECT("XUDPIN");
extern	void	udp_init(void)                                          SECT("XUDPINIT");
extern	int32	udp_recv(uint32, uint16, uint16, char *, int32, uint32) SECT("XUDPRECV");
extern	int32	udp_recvaddr(uint32 *, uint16*, uint16, char *, int32, uint32) SECT("XUDPRCVA");
extern	status	udp_register(uint32, uint16, uint16)                    SECT("XUDPREG");
extern	status	udp_send(uint32, uint16, uint32, uint16, char *, int32) SECT("XUDPSEND");
extern	status	udp_release(uint32, uint16, uint16)                     SECT("XUDPRLSE");

/* in file unsleep.c */
extern	syscall	unsleep(pid32)                                          SECT("XUNSLEEP");

/* in file userret.c */
extern	void	userret(void)                                           SECT("XUSERRET");

/* in file wait.c */
extern	syscall	wait(sid32)                                             SECT("XWAIT");

/* in file wakeup.c */
extern	void	wakeup(void)                                            SECT("XWAKEUP");

/* in file write.c */
extern	syscall	write(did32, char *, uint32)                            SECT("XWRITE");

/* in file xdone.c */
extern	void	xdone(void)                                             SECT("XXDONE");

/* in file yield.c */
extern	syscall	yield(void)                                             SECT("XYIELD");

/* in file dflClose.c */
extern	devcall	dflClose(struct dentry *)                               SECT("XDFLCLS");

/* in file dflControl.c */
extern	devcall	dflControl(struct dentry *, int32, int32, int32)        SECT("XDFLCNTL");

/* in file dflInit.c */
extern	devcall	dflInit(struct dentry *)                                SECT("XDFLINIT");

/* in file dflRead.c */
extern	devcall	dflRead(struct dentry *, char *, int32)                 SECT("XDFLREAD");

/* in file dflSeek.c */
extern	devcall	dflSeek(struct dentry *, uint32)                        SECT("XDFLSEEK");

/* in file dflWrite.c */
extern	devcall	dflWrite(struct dentry *, char *, int32)                SECT("XDFLWRIT");




/* NETWORK BYTE ORDER CONVERSION NOT NEEDED ON A BIG-ENDIAN COMPUTER */
#define	htons(x)	(x)
#define	htonl(x)	(x)
#define	ntohs(x)	(x)
#define	ntohl(x)	(x)

#endif /* PROTOTYPES_H */
