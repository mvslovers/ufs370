#include "ufs/disk.h"

/* check super block for sane values, return true if not sane */
INT32 ufs_disk_check_super(UFSDISK *disk, UFSSB *sb, int quiet)
{
    INT32   errors  = 0;
    char    *buf    = NULL;

	if (sb->volume_size < 128) {
        /* smaller than 64KB.   */
        if (!quiet) ufs_panic("%s volume size %u is too small", __func__, sb->volume_size);
        errors++;
	}

	if (sb->datablock_start_sector > sb->volume_size) {
        /* invalid              */
        if (!quiet) ufs_panic("%s data block start sector %u beyond volume size %u",
             __func__, sb->datablock_start_sector, sb->volume_size);
        errors++;
	}

    if (sb->nfreeinode > UFS_MAX_FREEINODE) {
        /* not likely           */
        if (!quiet) ufs_panic("%s free inode count %u greater than maximum free inode count %u",
             __func__, sb->nfreeinode, UFS_MAX_FREEINODE);
        errors++;
    }

    if (sb->nfreeblock > UFS_MAX_FREEBLOCK) {
        /* not likely           */
        if (!quiet) ufs_panic("%s free block count %u greater than maximum free block count %u",
             __func__, sb->nfreeblock, UFS_MAX_FREEBLOCK);
        errors++;
    }

    if (sb->update_time < 0) {
        /* time underflow       */
        if (!quiet) ufs_panic("%s update time %d less than 0", __func__, sb->update_time);
        errors++;
    }

    if (sb->total_freeblock > sb->volume_size) {
        if (!quiet) ufs_panic("%s total free blocks %u greater than volume size %u",
             __func__, sb->total_freeblock, sb->volume_size);
        errors++;
    }

    if ((sb->nfreeinode == 0) && (sb->nfreeblock == 0) &&
		(sb->total_freeblock == 0) && (sb->total_freeinode == 0)) {
        if (!quiet) ufs_panic("%s invalid super block, critical value all zero",
                         __func__);
        errors++;
    }

    if (!errors && disk) {
        /* try to read the last block of the disk dataset */
        buf = ufs_disk_get_buf(disk);
        if (buf) {
            if (ufs_disk_read(disk, buf, sb->volume_size - 1)) {
                if (!quiet) ufs_panic("%s invalid super block, last block %u not accessable",
                                 __func__, sb->volume_size - 1);
                errors++;
            }
            ufs_disk_free_buf(disk, buf);
        }

    }

	return errors;
}
