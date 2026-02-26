#include <stdlib.h>
#include <string.h>
#include <clibwto.h>
#include <osio.h>
#include <racf.h>
#include "ufs/fs.h"


static int fill_disk(const char *ddname, UINT32 blksize, UINT32 *blocks, int quiet, int verbose);
static int format_disk(const char *ddname, UINT32 blksize, UINT32 total_blocks, float pctiblk, int quiet, int verbose);
static int format_root(const char *ddname, float pctiblk, int quiet, int verbose);
static int format_report(UFSDISK *disk, float pctiblk, UFSMIN *minode);

static const char *help_text[] = {
    "Help Text",
    " ",
    "This program is used to format a dataset that will be used as a",
    "Unix (like) File System (UFS).",
    " ",
    "The dataset is initially filled with all 0 bytes and then formatted",
    "with a root \"/\" directory.",
    " ",
    "The resulting dataset has a block size of 512 bytes or more and must",
    "be a multiple of 512 bytes per block (512, 1024, 1536, 2048,...) up to",
    "8192 bytes. If not requested otherwise, a block size of 1024 is used.",
    " ",
    "The dataset can reside on any of the DASD units supported by MVS 3.8",
    "up to 3390 single density drives (3390-1). If the dataset will be on",
    "a 3390 DASD unit then a block size of 4096 is recommended.",
    " ",
    "When allocating a new dataset to be formatted, only the primary space",
    "of the dataset will be used. Do not specify secondary extents as these",
    "will not be used or formatted by this program.",
    " ",
    "Format Parameters (upper or lower case):",
    " -Blksize nn   Blocksize (default is 1024)",
    " -Ddname name  Name of DD to be formatted (default is DISKFILE)",
    " -Help or ?    Display this help text",
    " -Inodes pct   Percentage of blocks to be index nodes (default is 10.0)",
    " -Quiet        Suppress most output including format report",
    " -Verbose      Produce extra messages for each phase",
    " ",
    "Examples",
    " Format a dataset associated with //TESTFILE DD ... with a block",
    " size of 4096 bytes and 12.5 percent index nodes:",
    " //MYSTEP   EXEC PGM=FORMAT,PARM='-d TESTFILE -b 4096 -i 12.5'",
    "or",
    " //MYSTEP   EXEC PGM=FORMAT,PARM='DD TESTFILE BLKSIZE 4096 INODES 12.5'",
    " ",
    " Format the dataset associated with //DISKFILE DD ... with block",
    " size 1024 and 10.0 percent index nodes. Suppress most messages",
    " and bypass creation of the format report:",
    " //MYSTEP   EXEC PGM=FORMAT,PARM='-q'",
    "or",
    " //MYSTEP   EXEC PGM=FORMAT,PARM='QUIET'",
    " ",
    "Sample Step JCL:",
    " //FORMAT   EXEC PGM=FORMAT,PARM='-BLKSIZE 4096'",
    " //STEPLIB  DD DISP=SHR,DSN=*HLQ*.UFS.LINKLIB",
    " //SYSTERM  DD SYSOUT=*              STDERR",
    " //SYSPRINT DD SYSOUT=*              STDOUT (format report)",
    " //SYSIN    DD DUMMY                 STDIN",
    " //SYSABEND DD SYSOUT=*",
    " //DISKFILE DD DISP=OLD,DSN=*HLQ*.UFS.DISKFILE",
    " ",
    "End Help",
    NULL
};

__asm__("\n&FUNC    SETC 'main'");
int main(int argc, char **argv)
{
    int         rc          = 0;
    char        ddname[12]  = "DISKFILE";
    float       pctiblk     = 10.0;
    UINT32      blksize     = 1024;
    UINT32      blocks      = 0;
    int         quiet       = 0;
    int         verbose     = 0;
    int         i, j;
    int         len;

    for(i=1; i <argc; i++) {
        char *p = argv[i];

        len = strlen(p);
        if (*p=='-') {
            /* we found a switch character */
            p++;    /* skip over switch character */
            len--;  /* decrement the parm length */
        }

        if (strncmpi("help", p, len)==0 || p[0]=='?') {
            for(j=0; help_text[j]; j++) {
                wtof("%s", help_text[j]);
            }
            rc = 4;
            goto quit;  /* don't do anything else */
        }
        if (strncmpi("quiet", p, len)==0) {
            quiet = 1;
            continue;
        }
        if (strncmpi("verbose", p, len)==0) {
            verbose = 1;
            continue;
        }
        if (strncmpi("blksize", p, len)==0) {
            i++;    /* get next parm */
            if (i < argc && argv[i][0]!='-') {
                p = argv[i];
                blksize = strtoul(p,NULL,10);
                if (blksize < 512) {
                    wtof("UFSFMT01E Requested blksize %u must be 512 or greater", blksize);
                    rc = 8;
                }
                if (blksize & (512-1)) {
                    wtof("UFSFMT02E Requested blksize %u must be multiple of 512 bytes", blksize);
                    rc = 8;
                }
                if (blksize > 8192) {
                    wtof("UFSFMT03E Requested blksize %u greater than maximum of 8192", blksize);
                    rc = 8;
                }
            }
            else {
                wtof("UFSFMT04W Missing blksize value. Default of %u will be used", blksize);
                if (!rc) rc = 4;
                i--;
            }
            continue;
        }
        if (strncmpi("inodes", p, len)==0) {
            i++;    /* get next parm */
            if (i < argc && argv[i][0]!='-') {
                p = argv[i];
                pctiblk = atof(p);
                if (pctiblk < 1.0) {
                    wtof("UFSFMT05E Requested inodes percent %.1f must be 1.0 or greater", pctiblk);
                    rc = 8;
                }
                if (pctiblk > 100.0) {
                    wtof("UFSFMT06E Requested inodes percent %.1f greater than 100.0", pctiblk);
                    rc = 8;
                }
            }
            else {
                wtof("UFSFMT07W Missing inodes percentage value. Default of %.1f will be used", pctiblk);
                if (!rc) rc = 4;
                i--;
            }
            continue;
        }
        if (strncmpi("ddname", p, len)==0) {
            i++;    /* get next parm */
            if (i < argc && argv[i][0]!='-') {
                p = argv[i];
                len = strlen(p);
                if (len < 1 || len > 8) {
                    wtof("UFSFMT08E Invalid ddname length %d", len);
                    rc = 8;
                }
                else {
                    for(j=0; j < len; j++) {
                        if (isalnum(p[j]) || strchr("@#$",p[j])) {
                            ddname[j] = toupper(p[j]);
                        }
                        else {
                            wtof("UFSFMT09E Invalid character '%c' in ddname parameter", p[j]);
                            rc = 8;
                            break;
                        }
                    }
                    ddname[j] = 0;
                }
            }
            else {
                wtof("UFSFMT10W Missing ddname value. Default of %s will be used", ddname);
                if (!rc) rc = 4;
                i--;
            }
            continue;
        }
        wtof("UFSFMT11W Unknown parameter \"%s\"", p);
        if (!rc) rc = 4;
    }
    if (verbose) wtof("UFSFMT19I Parse of parameters ended with rc=%d", rc);
    if (rc > 4) goto quit;

    if (verbose) wtof("UFSFMT20I Preparing to initialize dataset for DD %s", ddname);
    rc = fill_disk(ddname, blksize, &blocks, quiet, verbose);
    if (verbose) wtof("UFSFMT39I Initialize ended with rc=%d", rc);
    if (rc) goto quit;

    if (verbose) wtof("UFSFMT40I Preparing to format the dataset as a file system");
    rc = format_disk(ddname, blksize, blocks, pctiblk, quiet, verbose);
    if (verbose) wtof("UFSFMT59I Dataset format ended with rc=%d", rc);
    if (rc) goto quit;

    if (verbose) wtof("UFSFMT60I Preparing to format the file system root directory");
    rc = format_root(ddname, pctiblk, quiet, verbose);
    if (verbose) wtof("UFSFMT79I Root directory format ended with rc=%d", rc);
    if (rc) goto quit;

quit:
    if (verbose) wtof("UFSFMT99I %s ending with rc=%d", argv[0], rc);
    return rc;
}

__asm__("\n&FUNC    SETC 'fill_disk'");
static int fill_disk(const char *ddname, UINT32 blksize, UINT32 *blocks, int quiet, int verbose)
{
    int         rc      = 8;
    char        *buf    = (char*)0;
    DCB         *dcb    = (DCB*)0;
    DECB        decb    = {0};
    JFCB        jfcb;
    UINT32      i;

    *blocks = 0;

    if (blksize & (512-1)) {
        /* blksize *should* be a multiple of 512 */
        if (!quiet) wtof("UFSFMT21E Requested blocksize not a multiple of 512, operation canceled");
        goto quit;
    }

    dcb = osbdcb(ddname, NULL);
    if (!dcb) {
        if (!quiet) wtof("UFSFMT22E unable to allocate OUTPUT DCB for DD %s", ddname);
        goto quit;
    }

    buf = calloc(1, blksize);
    if (!buf) {
        if (!quiet) wtof("UFSFMT23E Unable to allocate %d byte buffer", blksize);
        goto quit;
    }

    /* open BSAM dataset in "read" mode */
    rc = osbopen(dcb, 0, "r");
    if (rc) {
        if (!quiet) wtof("UFSFMT24E Unable to open DD %s for READ", ddname);
        goto quit;
    }

    /* get the Job File Control Block */
    rdjfcb(dcb, &jfcb);

    /* If the dataset is allocated DISP=SHR we don't want to continue */
    if (jfcb.jfcbind2 & JFCSHARE) {
        /* DISP=SHR allocation of this logical disk */
        rc = 8;
        if (!quiet) wtof("UFSFMT27E Unable to process DD %s, DISP=OLD required for FORMAT", ddname);
        /* Note: we're doing this here because in later format steps that write specific blocks
        ** the internal IO functions check the disk->readonly flag and will prohibit writes
        ** to logical disk with this flag set.  The readonly flag is set by the low level
        ** UFSDISK based functions when DISP=SHR is detected at open time.
        */
        quiet = 1;  /* suppress the UFSFMT26I Initialized... message below */
        goto quit;
    }
    /* close the BSAM dataset we opened in READ mode */
    osbclose(dcb, NULL, 0, 0);

    /* set block size in DCB */
    dcb->dcbblksi = (unsigned short)blksize;
    dcb->dcblrecl = dcb->dcbblksi;

    /* open BSAM dataset in "load" mode */
    rc = osbopen(dcb, 0, "load");
    if (rc) {
        if (!quiet) wtof("UFSFMT24E Unable to open DD %s for OUTPUT", ddname);
        goto quit;
    }

    /* we only want to initialize the primary extent, skip secondary extents even if allocated */
    /* rc=4 is end of track, rc=8 is end of extent, rc=12 is write error, no more tracks (SB37) */
    for(i=1; rc<=4; i++) {
        rc = oswrite(&decb, dcb, buf, blksize);

        /* check for real write errors */
        if (rc > 8) {
            if (!quiet) wtof("UFSFMT25E oswrite returned rc=%d for block %d", rc, i);
            goto quit;
        }

        /* we have to CHECK to make sure the write completed okay */
        oscheck(&decb);
    }

    /* success */
    *blocks = --i;
    rc = 0;


quit:
    if (!quiet) wtof("UFSFMT26I Initialized %u blocks (%.2f MB)",
                     *blocks, (*blocks * blksize) / 1048576.0);
    if (buf) free(buf);
    if (dcb) osbclose(dcb, NULL, 1, 0);

    return rc;
}

__asm__("\n&FUNC    SETC 'format_disk'");
static int format_disk(const char *ddname, UINT32 blksize, UINT32 total_blocks, float pctiblk, int quiet, int verbose)
{
    int         rc      = 8;
    char        *buf    = (char*)0;
    DCB         *dcb    = (DCB*)0;
    DECB        decb    = {0};
    UFSBOOT     boot    = {0};
    UFSBOOTE    boote   = {0};
    UFSSB       sb      = {0};
    UFSFB       fb      = {0};
    UINT32      inodes  = 0;
    UINT32      freeblks= 0;
    UFSDIN      *dinode;
    UFSFB       *fbp;
    UINT32      block;
    UINT32      blocks;
    UINT32      blockchain = 0;
    UINT32      n;
    UINT32      i;
    time64_t    now;

    /* disk layout
    ** *------------------------------------*
    ** * blk#0  boot block | extension
    ** *------------------------------------*
    ** * blk#1  super block
    ** *------------------------------------*
    ** * blk#2  through n, inode blocks
    ** *------------------------------------*
    ** * ... inode blocks
    ** *------------------------------------*
    ** * blk#n  first data block
    ** *------------------------------------*
    ** * ...data blocks
    ** *------------------------------------*
    **/

    /* calculate number of inode blocks we'll allocate */
    blocks  = (unsigned) ((total_blocks / UFS_INODE_PER_BLOCK(blksize)) * (pctiblk / 100.0) + 0.50);
    if (blocks < 2) blocks = 2;

    /* block#0 boot block */
    boot.type       = UFS_DISK_TYPE_UFS;
    boot.check      = ~boot.type;
    boot.blksize    = blksize;

#if 1
    /* block#0 boot extension */
    boote.version   = UFS_DISK_BOOT_1;
    time64(&now);
    boote.update_time = now;
    boote.create_time = now;
#endif

    /* block#1 super block */
    sb.datablock_start_sector   = UFS_ILIST_SECTOR + blocks;
    sb.volume_size              = total_blocks;

	/* we don't have room in the superblock to expand the update_time or create_time
	** so we'll have to leave these as-is until we can create a new version with a larger
	** superblock > 512 bytes.
	*/
#if 1
    sb.update_time              = (time_t)0;    /* no longer used. see boot extension */
#else
    sb.update_time              = time(NULL);	/* this will break after year 2105 */
#endif
	sb.total_freeblock          = total_blocks - sb.datablock_start_sector;
    sb.total_freeinode          = UFS_ILISTBLK_MAX(blksize,sb.datablock_start_sector) - 2 /* reserved inodes */;
    sb.create_time              = sb.update_time;
    sb.inodes_per_block         = UFS_INODE_PER_BLOCK(blksize);

	for(n=blksize; n; n=(n>>1)) {
        if (n&1==1) break;
        sb.blksize_shift++;
	}

    sb.ilist_sector             = UFS_ILIST_SECTOR;

    /* open the dataset for BDAM processing */
    dcb = osddcb(ddname);
    if (!dcb) {
        if (!quiet) wtof("UFSFMT41E unable to allocate DCB for BDAM DD %s", ddname);
        goto quit;
    }

    rc = osdopen(dcb, 0);
    if (rc) {
        if (!quiet) wtof("UFSFMT42E Unable to open DD %s for UPDATE, rc=%d", ddname, rc);
        goto quit;
    }

    buf = calloc(1, blksize);
    if (!buf) {
        if (!quiet) wtof("UFSFMT43E Unable to allocate %d byte buffer", blksize);
        goto quit;
    }

    /* write the boot block */
    memcpy(buf, &boot, sizeof(boot));
#if 1
    /* append boot extension to buffer */
    memcpy(buf+sizeof(UFSBOOT), &boote, sizeof(UFSBOOTE));
#endif
    rc = osdwrite(&decb, dcb, buf, blksize, UFS_BOOTBLOCK_SECTOR);
    oscheck(&decb);
    if (rc) {
        if (!quiet) wtof("UFSFMT44E Write of boot block failed with rc=%d", rc);
        goto quit;
    }

    /* write the *partial* superblock */
    memcpy(buf, &sb, sizeof(sb));
    rc = osdwrite(&decb, dcb, buf, blksize, UFS_SUPERBLOCK_SECTOR);
    oscheck(&decb);
    if (rc) {
        if (!quiet) wtof("UFSFMT45E Write of super block failed with rc=%d", rc);
        goto quit;
    }

    /* write the inodes */
    /* note: the first inode(#0) and inode(#1) are never used */
    block = UFS_ILIST_SECTOR;

    for(block=0; block < blocks; block++) {
        memset(buf, 0xff, blksize);
        dinode = (UFSDIN *)buf;

        for(i=0; i < sb.inodes_per_block; i++) {
            if ((block == 0) && (i<=1)) {
                /* these inodes are reserved */
            }
            else {
                /* normal unassigned inodes */
                memset(dinode, 0, sizeof(*dinode));
                inodes++;   /* free inodes counter */
            }
            dinode++;   /* next inode in block */
        }
        rc = osdwrite(&decb, dcb, buf, blksize, UFS_ILIST_SECTOR + block);
        oscheck(&decb);
        if (rc) {
            if (!quiet) wtof("UFSFMT46E write of INODE block %u failed with rc=%d",
                             UFS_ILIST_SECTOR + block, rc);
            goto quit;
        }
    }
    sb.total_freeinode = inodes;

    /* create free block chain in super block area */
    for(block=sb.datablock_start_sector; block < total_blocks; block++) {
        sb.freeblock[sb.nfreeblock++] = block;
        if (sb.nfreeblock==UFS_MAX_FREEBLOCK) {
            freeblks += sb.nfreeblock;
            break;
        }
    }

    /* build free data block chain */
    blockchain = sb.datablock_start_sector;
    for(block++; block < total_blocks; block++) {
        fb.freeblock[fb.nfreeblock++] = block;
        if (fb.nfreeblock==UFS_MAX_FREEBLOCK) {
            freeblks += fb.nfreeblock;
            memset(buf, 0, blksize);
            memcpy(buf, &fb, sizeof(fb));
            rc = osdwrite(&decb, dcb, buf, blksize, blockchain);
            oscheck(&decb);
            if (rc) {
                if (!quiet) wtof("UFSFMT47E write of free block %u failed with rc=%d",
                                 fb.freeblock[0], rc);
                goto quit;
            }
            blockchain = fb.freeblock[0];
            memset(&fb, 0, sizeof(fb));
        }
    }
    if (fb.nfreeblock) {
        freeblks += fb.nfreeblock;
        memset(buf, 0, blksize);
        memcpy(buf, &fb, sizeof(fb));
        rc = osdwrite(&decb, dcb, buf, blksize, blockchain);
        oscheck(&decb);
        if (rc) {
            if (!quiet) wtof("UFSFMT48E write of last freeblock %u failed with rc=%d",
                             fb.freeblock[0], rc);
            goto quit;
        }
    }
    sb.total_freeblock = freeblks;

#if 0
    /* load the super block free data cache */
    rc = osdread(&decb, dcb, buf, blksize, sb.datablock_start_sector);
    oscheck(&decb);
    if (rc) {
        if (!quiet) wtof("UFSFMT49E read of super block failed with rc=%d", rc);
        goto quit;
    }
    fbp = (UFSFB *)buf;
	memcpy(sb.freeblock, fbp->freeblock, sizeof(block) * fbp->nfreeblock);
    sb.nfreeblock = fbp->nfreeblock;
#endif

    /* we'll prime these so they will be allocated for the root directory allocation */
    sb.nfreeinode = 1;
    sb.freeinode[0] = UFS_ROOT_INODE;

    /* write the superblock */
    memcpy(buf, &sb, sizeof(sb));
    rc = osdwrite(&decb, dcb, buf, blksize, UFS_SUPERBLOCK_SECTOR);
    oscheck(&decb);
    if (rc) {
        if (!quiet) wtof("UFSFMT50E write of super block failed with rc=%d", rc);
        goto quit;
    }

    /* at this point the disk is formatted, however we still need to create the root directory */
    if (!quiet) {
        blocks = sb.datablock_start_sector - UFS_ILIST_SECTOR;
        wtof("UFSFMT51I Formatted %u INDEX blocks with %u INODES", blocks, sb.total_freeinode);
        wtof("UFSFMT52I Formatted %u DATA blocks", sb.total_freeblock);
        wtof("UFSFMT53I Formatted %u total blocks", sb.volume_size);
    }

quit:
    if (buf) free(buf);
    if (dcb) osdclose(dcb, 1);

    return rc;
}

__asm__("\n&FUNC    SETC 'format_root'");
static int format_root(const char *ddname, float pctiblk, int quiet, int verbose)
{
    int     rc          = 8;
    ACEE    *acee       = racf_get_acee();
    char    *buf        = NULL;
    UFSDISK *disk       = NULL;
    UFSVDISK *vdisk     = NULL;
    UFSDIR  *dir        = NULL;
    UFSDIN  *dinode     = NULL;
    UINT32  ino         = 0;
    UINT32  block       = 0;
    UFSIO   io          = {0, (void*)ufs_disk_read, (void*)ufs_disk_read_n, (void*)ufs_disk_write, (void*)ufs_disk_write_n};
    UFSMIN  minode      = {0};
    UINT32  n;

#if 0
    wtodumpf(acee, sizeof(ACEE), "ACEE");
    if (acee->aceecgrp) {
        CONNGRP *grp = acee->aceecgrp;
        for (grp = acee->aceecgrp; grp; grp=grp->next) {
            wtodumpf(grp, sizeof(CONNGRP), "CONNGRP");
        }
    }
#endif

    disk = ufs_disk_open(ddname, 0);    /* 0==don't check super block */
    if (!disk) {
        if (!quiet) wtof("UFSFMT61E Unable to open DD %s as a disk", ddname);
        goto quit;
    }

    buf = calloc(1, disk->blksize);
    if (!buf) {
        if (!quiet) wtof("UFSFMT62E Unable to allocate %d byte buffer", disk->blksize);
        goto quit;
    }

    io.ctx = disk;
    vdisk = ufs_vdisk_new(disk, &io);
    if (!vdisk) {
        if (!quiet) wtof("UFSFMT63E Unable to open VDISK for DD %s", ddname);
        goto quit;
    }
#if 0
    wtof("%s sb.modified=%d, sb->nfreeblock=%u", __func__, disk->sb.modified, disk->sb.nfreeblock);
    for(n=0; n < disk->sb.nfreeblock; n++) {
        wtof("%s free block %u is %u", __func__, n, disk->sb.freeblock[n]);
    }
#endif
    rc = ufs_inode_allocate(vdisk, &ino);
    if (rc) {
        if (!quiet) wtof("UFSFMT64E Unable to allocate INODE for root directory, rc=%d", rc);
        goto quit;
    }
    if (verbose) wtof("UFSFMT65I Allocated INODE %u for directory", ino);

    if (ino != UFS_ROOT_INODE) {
        if (!quiet) wtof("UFSFMT66E Error incorrect INODE %u, expecting INODE %u for root directory",
                         ino, UFS_ROOT_INODE);
        ufs_inode_deallocate(vdisk,ino);
        rc = 8;
        goto quit;
    }

    rc = ufs_datablock_allocate(vdisk, &block);
    if (rc) {
        if (!quiet) wtof("UFSFMT67E Unable to allocate DATA block for root directory, rc=%d", rc);
        ufs_inode_deallocate(vdisk,ino);
        goto quit;
    }
    if (verbose) wtof("UFSFMT68I Allocated DATA block %u for directory", block);

    memset(&minode, 0, sizeof(minode));
    minode.inode_number     = ino;
    minode.vdisk            = vdisk;

    dinode                  = &minode.dinode;
    dinode->mode            = UFS_IFDIR | UFS_UMASK;
    if (acee && memcmp(acee->aceeacee, "ACEE", 4)==0) {
        /* get owner and group from security environment */
        memcpyp(dinode->owner, sizeof(dinode->owner), &acee->aceeuser[1], acee->aceeuser[0], 0);
        memcpyp(dinode->group, sizeof(dinode->group), &acee->aceegrp[1], acee->aceegrp[0], 0);
    }
    else {
        /* plug in some *safe* defaults */
        strcpy(dinode->owner, "HERC01");
        strcpy(dinode->group, "STGADMIN");
    }
    dinode->filesize        = sizeof(UFSDIR)*2;
    dinode->nlink           = 2;    /* "." and ".." */
    dinode->addr[0]         = block;
    dinode->ctime.v2        = mtime64(NULL);
    dinode->atime           = dinode->ctime;
    dinode->mtime           = dinode->ctime;
    rc = ufs_inode_writeback(&minode);
    if (rc) {
        if (!quiet) wtof("UFSFMT69E Unable to save root directory INODE %u", ino);
        goto quit;
    }

    dir = (UFSDIR*)buf;
    dir[0].inode_number     = ino;  /* the "root" node itself */
    strcpy(dir[0].name, ".");
    dir[1].inode_number     = ino;  /* the "root" parent always refers to itself */
    strcpy(dir[1].name, "..");
    rc = ufs_vdisk_write(vdisk, buf, block);
    if (rc) {
        if (!quiet) wtof("UFSFMT70E Unable to write root directory DATA block %u", block);
        goto quit;
    }

    if (!quiet) wtof("UFSFMT71I Root directory created with owner=\"%s\", group=\"%s\", mode=%o",
                      dinode->owner, dinode->group, UFS_UMASK);

    /* wtof("%s sb.modified=%d, sb->nfreeblock=%u", __func__, disk->sb.modified, disk->sb.nfreeblock); */
    if (disk->sb.modified) {
        /* save the super lock */
        rc = ufs_disk_write_super(disk, &disk->sb);
        if (rc) {
            wtof("UFSFMT71E Write of super block failed, rc=%d", rc);
            goto quit;
        }
    }

    if (!quiet) format_report(disk, pctiblk, &minode);

quit:
    if (verbose) wtof("UFSFMT72I Root directory creation ended with rc=%d", rc);
    if (buf) free(buf);
    if (vdisk) ufs_vdisk_free(&vdisk);
    if (disk) ufs_disk_close(&disk);

    return rc;
}

static int format_report(UFSDISK *disk, float pctiblk, UFSMIN *minode)
{
    UINT32  blksize         = disk->blksize;
    const char *ddname      = disk->ddname;
    const char *dsname      = disk->dsname;
    UFSSB   *sb             = &disk->sb;
    time64_t now            = time64(NULL);
    UINT32  index_blocks    = sb->datablock_start_sector - UFS_ILIST_SECTOR;
    UINT32  data_blocks     = sb->volume_size - sb->datablock_start_sector;
    UINT32  res_blocks      = UFS_ILIST_SECTOR;
    UINT32  index_nodes     = index_blocks * sb->inodes_per_block;
    UINT32  index_used      = index_nodes - sb->total_freeinode;
    UINT32  data_used       = data_blocks - sb->total_freeblock;

    printf("                            F O R M A T   R E P O R T          %s", ctime64(&now));
    printf(" \n");
    printf("DD Name                     %s\n", ddname);
    printf("Dataset Name                '%s'\n", dsname);
    printf(" \n");
    printf("Block Size                  %u\n", blksize);
    printf("Blocks Formatted            %u\n", sb->volume_size);
    printf("Bytes Formatted             %.2fMB\n", (blksize * sb->volume_size) / 1048576.0);
    printf(" \n");
    printf("Index Nodes Pct             %.2f%%\n", pctiblk);
    printf(" \n");
    printf("Index Nodes Per Block       %u\n", sb->inodes_per_block);
    printf("Index Node Size             %u\n", sizeof(UFSDIN));
    printf("Index Blocks Total          %u\n", index_blocks);
    printf(" \n");
    printf("Index Nodes Used            %u\n", index_used);
    printf("Index Nodes Available       %u\n", sb->total_freeinode);
    printf("Index Nodes Total           %u\n", index_nodes);
    printf(" \n");
    printf("Index Nodes Used Pct        %.2f%%\n", (index_used*100.0) / index_nodes);
    printf("Index Nodes Available Pct   %.2f%%\n", (sb->total_freeinode*100.0) / index_nodes);
    printf(" \n");
    printf("Data Blocks Used            %u\n", data_used);
    printf("Data Blocks Used Pct        %.2f%%\n", (data_used*100.0) / data_blocks);
    printf("Data Blocks Available Pct   %.2f%%\n", (sb->total_freeblock*100.0) / data_blocks);
    printf("Data Blocks Total           %u\n", data_blocks);
    printf(" \n");
    printf("Directory Name Max Size     %u\n", UFS_NAME_MAX);
    printf("Directory Record Size       %u\n", sizeof(UFSDIR));
    printf("Directory Records Per Block %u\n", blksize / sizeof(UFSDIR));
    printf(" \n");
    printf("Reserved Blocks             %u  (boot, super block)\n", res_blocks);
    printf(" \n");
    printf("Root Directory Inode        %u\n", minode->inode_number);
    printf("Root Directory Data Block   %u\n", minode->dinode.addr[0]);
    printf("Root Directory Owner        %s\n", minode->dinode.owner);
    printf("Root Directory Group        %s\n", minode->dinode.group);
    printf("Root Directory UMASK        %03o  (owner, group, world)\n", minode->dinode.mode & 0777);
    printf("Root Directory Links        %u\n", minode->dinode.nlink);
    printf("Root Directory File Size    %u\n", minode->dinode.filesize);

    return 0;
}
