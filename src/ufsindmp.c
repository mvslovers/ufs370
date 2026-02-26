#include "ufs/inode.h"
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
__asm__("\n&FUNC    SETC 'ufs_inode_dump'");
void ufs_inode_dump(UFSMIN *minode)
{
    UFSDIN  *dinode = &minode->dinode;
    char    *type = 0;
#if 1
    char    caller[64] = "";

    __caller(caller);
    wtof("%s for caller %s", __func__, caller);
#endif

    if (!type && minode->type & MINODE_TYPE_FILE) type = "FILE";
    if (!type && minode->type & MINODE_TYPE_FSROOT) type = "FSROOT";
	if (!type && minode->type & MINODE_TYPE_MOUNTPOINT) type = "MOUNTPOINT";
    if (!type && minode->type & MINODE_TYPE_DIR) type = "DIR";
    if (!type && ufs_inode_isdir(dinode)) type = "DIR";
    if (!type && ufs_inode_isfile(dinode)) type = "FILE";
    if (!type) type = "????";

    wtof("inode %u, usecount=%u, flags=%02X, type=%s",
         minode->inode_number, minode->usecount, minode->flags, type);

    wtof("   nlink:%u mode:%06o",
         dinode->nlink, dinode->mode);
    wtof("   uid:%s gid:%s filesize:%u bytes",
	     dinode->owner, dinode->group, dinode->filesize);

	if (dinode->ctime.v1.useconds < 1000000) {
		/* v1 timestamp */
		wtof("   atime %d mtime %d ctime %d",
			dinode->atime.v1.seconds, dinode->mtime.v1.seconds, dinode->ctime.v1.seconds);
	}
	else {
		wtof("   atime %lld mtime %lld ctime %lld",
			dinode->atime.v2, dinode->mtime.v2, dinode->ctime.v2);
	}

	if (dinode->ctime.v1.useconds < 1000000) {
		/* v1 timestamp */
		wtof("    atime %s", ctime((time_t*)&dinode->atime.v1.seconds));
	}
	else {
		wtof("    atime %s", uctime64((utime64_t*)&dinode->atime.v2));
	}
	if (dinode->mtime.v1.useconds < 1000000) {
		/* v1 timestamp */
		wtof("    mtime %s", ctime((time_t*)&dinode->mtime.v1.seconds));
	}
	else {
		wtof("    mtime %s", uctime64((utime64_t*)&dinode->mtime.v2));
	}
	if (dinode->ctime.v1.useconds < 1000000) {
		/* v1 timestamp */
		wtof("    ctime %s", ctime((time_t*)&dinode->ctime.v1.seconds));
	}
	else {
		wtof("    ctime %s", uctime64((utime64_t*)&dinode->ctime.v2));
	}
#if 0
    if (ufs_inode_iscdev(dinode) || ufs_inode_isbdev(dinode)) {
        wtof("device:%u/%u", (minode->device >> 8) & 0xFF, minode->device & 0xFF);
    }
#endif
    wtof(" ");
}
