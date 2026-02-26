#include "ufs/disk.h"

/* open a formatted disk dataset for I/O */
__asm__("\n&FUNC    SETC 'ufs_disk_open'");
UFSDISK *ufs_disk_open(const char *ddname, int check_sb)
{
    UFSDISK     *disk   = calloc(1, sizeof(UFSDISK));
    char        *buf    = NULL;
    UFSBOOT     *boot;
    UFSBOOTE    *boote;
    UFSSB       *sb;
    JFCB        jfcb;
    int         i;
    INT32       rc;

    if (!disk) {
        wtof(UFSDK001E);
        goto quit;
    }

    /* initialize the disk handle */
    strcpyp(disk->eye, sizeof(disk->eye), UFSDISK_EYE, 0);
    strcpyp(disk->ddname, sizeof(disk->ddname), (void*)ddname, 0);

    /* allocate a BDAM DCB */
    disk->dcb = osddcb(disk->ddname);
    if (!disk->dcb) {
        wtof(UFSDK002E);
        goto failed;
    }

    /* open the BDAM dataset */
    rc = osdopen(disk->dcb, 0);
    if (rc) {
        wtof(UFSDK003E, disk->ddname);
        goto failed;
    }

    /* get the dataset name from the Job File Control Block */
    rdjfcb(disk->dcb, &jfcb);
    for(i=0; (jfcb.jfcbdsnm[i] > ' ') && (i < sizeof(disk->dsname)); i++) {
        disk->dsname[i] = jfcb.jfcbdsnm[i];
    }

    /* If the dataset is allocated DISP=SHR we don't allow updates */
    if (jfcb.jfcbind2 & JFCSHARE) {
        /* DISP=SHR allocation of this logical disk */
        disk->readonly = 0xFF;  /* set read only flag byte */
    }


    /* get the physical block size from the DCB */
    disk->blksize = disk->dcb->dcbblksi;

    /* we require the block size to be UFS_BSIZE */
    if (disk->blksize & (512-1)) {
        wtof(UFSDK004E, disk->dsname);
        goto failed;
    }

    /* allocate a buffer so we can read the super block */
    buf = calloc(1, disk->blksize);
    if (!buf) {
        wtof(UFSDK005E, disk->blksize);
        goto failed;
    }

    /* read the boot block (block 0) */
    rc = ufs_disk_read(disk, buf, UFS_BOOTBLOCK_SECTOR);
    if (rc) {
        wtof(UFSDK006E, disk->dsname);
        goto failed;
    }

    /* check for proper block size info */
    boot = (UFSBOOT *)buf;
    boote = (UFSBOOTE *)(buf + sizeof(UFSBOOT));
    if (boot->blksize != disk->blksize) {
        wtof(UFSDK007E, disk->dsname);
        goto failed;
    }
    if ((boot->type + boot->check) != 0xFFFF) {
        wtof(UFSDK008E, disk->dsname);
        goto failed;
    }

    /* copy boot block to disk handle */
    memcpy(&disk->boot, boot, sizeof(disk->boot));
    
#if 1
    /* allocate boot extension in disk handle */
    disk->boote = calloc(1, sizeof(UFSBOOTE));
    if (!disk->boote) {
        wtof(UFSDK005E, sizeof(UFSBOOTE));
        goto failed;
    }
    /* copy boot extension to disk handle */
    memcpy(disk->boote, boote, sizeof(UFSBOOTE));
#endif

    /* read the super block into our buffer */
    sb = (UFSSB *)buf;
    rc = ufs_disk_read_super(disk, sb);
    if (rc) {
        wtof(UFSDK009E, disk->dsname);
        goto failed;
    }

    if (check_sb) {
        /* check the super block */
        rc = ufs_disk_check_super(disk, sb, 0 /* 0=don't be quiet */);
        if (rc) {
            wtof(UFSDK010E, disk->dsname);
            goto failed;
        }
    }

    /* success, copy the super block to the disk handle */
    memcpy(&disk->sb, sb, sizeof(disk->sb));

#if 1
    /* point to copy of boot extension in disk handle */
    boote = disk->boote;

    /* if old format, update boot extension to new version */
    if (boote->version == UFS_DISK_BOOT_0) {
        sb = &disk->sb;

        /* update boot extension version */
        boote->version = UFS_DISK_BOOT_1;

        /* convert create_time */
        boote->create_time.u32[0] = 0;
        boote->create_time.u32[1] = sb->create_time;
        sb->create_time = (time_t)0;

        /* convert update_time */
        boote->update_time.u32[0] = 0;
        boote->update_time.u32[1] = sb->update_time;
        sb->update_time = (time_t)0;

        /* set superblock modified flag to force update at close */
        sb->modified = 1;
    }
#endif

quit:
    if (buf) free(buf);

    return disk;

failed:
    ufs_disk_close(&disk);
    goto quit;
}
