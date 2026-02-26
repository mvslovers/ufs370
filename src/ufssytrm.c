#include "ufs/sys.h"

__asm__("\n&FUNC    SETC 'ufs_sys_term'");
void ufs_sys_term(void)
{
    UFSSYS      *sys    = ufs_sys_get();    /* allocated by __wsaget()  */
    UFSPAGER    *pager;                     /* page caching handle      */
    UFSIO       *io;                        /* block level i/o vector   */
    UFSDISK     *disk;                      /* physical disk handle     */
    UFSVDISK    *vdisk;                     /* logical disk handle      */
    UFSMIN      *minode;
    UFSNAME     *name;
    UFSCWD      *cwd;
    UINT32      count;
    UINT32      n;

    if (!sys) goto quit;

    /* wtodumpf(sys, sizeof(UFSSYS), "%s UFSSYS", __func__); */
    if (memcmp(sys->eye, UFSSYS_EYE, sizeof(sys->eye))!=0) {
        /* already terminated */
        ufs_panic("%s UFSSYS handle already terminated", __func__);
        goto quit;
    }

    /* release cwds */
    if (sys->cwds) {
        /* wtof("%s sys->cwds=%08X", __func__, sys->cwds); */
        count = array_count(&sys->cwds);
        for(n=count; n > 0; n--) {
            cwd = array_del(&sys->cwds, n);
            if (!cwd) continue;
            ufs_cwd_free(&cwd);
        }
        array_free(&sys->cwds);
    }

    /* release the names */
    if (sys->names) {
        /* wtof("%s sys->names=%08X", __func__, sys->names); */
        count = array_count(&sys->names);
        for(n=count; n > 0; n--) {
            name = array_del(&sys->names, n);
            if (!name) continue;
            if (name->rep) free(name->rep);
            if (name->prefix) free(name->prefix);
            free(name);
        }
        array_free(&sys->names);
    }

	/* unmount all disk from mount points (paths) */
	if (sys->mountpoint) {
		ufs_unmount_all();
		array_free(&sys->mountpoint);
	}

    /* we have to release the file system root inode before we release the vdisk */
    if (sys->fsroot) {
        /* wtof("%s sys->fsroot=%08X", __func__, sys->fsroot); */
        minode = sys->fsroot;
        vdisk = minode->vdisk;
        if (minode->flags & MINODE_FLAG_DIRTY) {
            ufs_inode_writeback(minode);
        }
        ufs_inode_rel(minode);
        sys->fsroot = NULL;
    }

    /* free the vdisk handles */
    if (sys->vdisks) {
        /* wtof("%s sys->vdisks=%08X", __func__, sys->vdisks); */
        count = array_count(&sys->vdisks);
        for(n=count; n > 0; n--) {
            vdisk = array_del(&sys->vdisks, n);
            if (!vdisk) continue;
            ufs_vdisk_free(&vdisk);
        }
        array_free(&sys->vdisks);
    }

    /* flush the pagers to disk and free the pager handles */
    if (sys->pagers) {
        /* wtof("%s sys->pagers=%08X", __func__, sys->pagers); */
        count = array_count(&sys->pagers);
        for(n=count; n > 0; n--) {
            pager = array_del(&sys->pagers, n);
            if (!pager) continue;
            ufs_pager_free(&pager, 1 /* flush to disk */);
        }
        array_free(&sys->pagers);
    }

    /* free the i/o handles */
    if (sys->io) {
        /* wtof("%s sys->io=%08X", __func__, sys->io); */
        count = array_count(&sys->io);
        for(n=count; n > 0; n--) {
            io = array_del(&sys->io, n);
            if (!io) continue;
            ufs_io_free(&io);
        }
        array_free(&sys->io);
    }

    /* close the physical disk */
    if (sys->disks) {
        /* wtof("%s sys->disks=%08X", __func__, sys->disks); */
        count = array_count(&sys->disks);
        for(n=count; n > 0; n--) {
            disk = array_del(&sys->disks, n);
            if (!disk) continue;
            ufs_disk_close(&disk);
        }
        array_free(&sys->disks);
    }

    /* clear the sys handle */
    memset(sys, 0, sizeof(UFSSYS));

quit:
    return;
}
