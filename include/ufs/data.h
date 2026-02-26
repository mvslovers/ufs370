#ifndef UFS_DATA_H
#define UFS_DATA_H
/*-
 * Copyright (c) 2011 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by UCHIYAMA Yasushi.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include "ufs/types.h"      /* atomic types                             */
#include "ufs/time.h"       /* time functions                           */
#include "ufs/disk.h"       /* low level disk, boot block, super block  */
#include "ufs/vdisk.h"      /* virtual disk handle                      */
#include "ufs/inode.h"      /* index nodes                              */
#include "ufs/dir.h"        /* directory entry                          */

#define	UFS_ITERATOR_BREAK	(-1)
#define	UFS_ITERATOR_END	(-2)
#define	UFS_ITERATOR_ERROR	(-3)

typedef struct ufs_addr_map UFSADRM;
struct ufs_addr_map {
    int     level;      /* direct, index1, index2, index3 */
	UINT32  index[3];
};

INT32 ufs_datablock_number_sanity(UFSVDISK *vdisk, UINT32 block)                    asm("UFSDASAN");

INT32 ufs_datablock_allocate(UFSVDISK *vdisk, UINT32 *number)                       asm("UFSDAALC");

INT32 ufs_datablock_deallocate(UFSVDISK *vdisk, UINT32 block)                       asm("UFSDADAL");

INT32 ufs_datablock_foreach(UFSVDISK *vdisk, UFSMIN *minode,
    INT32 (*func)(UFSVDISK *, void *, UINT32, UINT32), void *ctx)                   asm("UFSDAFOR");

INT32 ufs_datablock_loop1(UFSVDISK *vdisk, UINT32 listblk, UINT32 *filesize,
    int (*func)(UFSVDISK *vdisk, void *, UINT32, UINT32), void *ctx)                asm("UFSDALP1");

INT32 ufs_datablock_loop2(UFSVDISK *vdisk, UINT32 listblk, UINT32 *filesize,
    int (*func)(UFSVDISK *vdisk, void *, UINT32, UINT32), void *ctx)                asm("UFSDALP2");

UINT32 ufs_datablock_link(UFSVDISK *vdisk, UINT32 listblk, int n)                   asm("UFSDALNK");

UINT32 ufs_datablock_last(UFSMIN *minode, UINT32 offset)                            asm("UFSDALST");

INT32 ufs_datablock_expand(UFSVDISK *vdisk, UFSMIN *minode, UINT32 size)            asm("UFSDAEXP");

INT32 ufs_datablock_contract(UFSVDISK *vdisk, UFSMIN *minode, UINT32 size)          asm("UFSDACON");

INT32 ufs_datablock_size_change(UFSVDISK *vdisk, UINT32 newsz, UFSMIN *minode)      asm("UFSDACHG");

INT32 ufs_datablock_addr(UFSVDISK *vdisk, UINT32 sz, UFSADRM *map)                  asm("UFSDAADR");

/* Fill free block to superblock cache. */
INT32 ufs_freeblock_update(UFSVDISK *vdisk, UINT32 blk)                             asm("UFSDAFUP");

UINT32 ufs_datablock_add_leaf(UFSVDISK *vdisk, UINT32 up, int idx)                  asm("UFSDAALF");

UINT32 ufs_datablock_unlink(UFSVDISK *vdisk, UINT32 idxblk, int n)                  asm("UFSDAULK");

UINT32 ufs_datablock_remove_leaf(UFSVDISK *vdisk, UINT32 up, int n)                 asm("UFSDARML");

UINT32 ufs_datablock_remove_self(UFSVDISK *vdisk, UINT32 up)                        asm("UFSDARMS");

void ufs_daddr_map_dump(UFSADRM *map)                                               asm("UFSDADMP");

#endif /* UFS_DATA_H */
