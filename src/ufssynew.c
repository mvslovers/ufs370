#include "ufs/sys.h"
#include "ieftiot.h"

static TIOTDD *find_dd(const char *ddname);

__asm__("\n&FUNC    SETC 'ufs_sys_new'");
UFSSYS *ufs_sys_new(void)
{
    UFSSYS      *sys    = ufs_sys_get();    /* allocated by __wsaget()  */
    UFSPAGER    *pager;                     /* page caching handle      */
    UFSIO       *io;                        /* block level i/o vector   */
    UFSDISK     *disk;                      /* physical disk handle     */
    UFSVDISK    *vdisk;                     /* logical disk handle      */
    char        ddname[12];                 /* disk dd name             */
    int         i;

    if (!sys) goto quit;

    if (memcmp(sys->eye, UFSSYS_EYE, sizeof(sys->eye))==0) {
        /* already initialized */
        ufs_panic("%s UFSSYS handle already initialized", __func__);
        goto quit;
    }

    /* put our eye catcher here */
    memcpy(sys->eye, UFSSYS_EYE, sizeof(sys->eye));

    /* mount "UFSDISK0" - "UFSDISK9" as disk */
    for(i=0; i < 10; i++) {
        sprintf(ddname, "UFSDISK%d", i);
        if (!find_dd(ddname)) continue;

        disk =  ufs_disk_open(ddname, 1);
        if (!disk) continue;

        array_add(&sys->disks, disk);

        pager = ufs_pager_new(disk, 20);
        if (pager) {
            array_add(&sys->pagers, pager);
            io = ufs_io_new(pager, ufs_pager_read, ufs_pager_read_n, ufs_pager_write, ufs_pager_write_n);
        }
        else {
            io = ufs_io_new(disk, ufs_disk_read, ufs_disk_read_n, ufs_disk_write, ufs_disk_write_n);
        }

        if (!io) {
            ufs_panic("%s ufs_io_new failure", __func__);
            ufs_sys_term();
            return NULL;
        }

        array_add(&sys->io, io);

        vdisk = ufs_vdisk_new(disk, io);
        if (!vdisk) {
            ufs_panic("%s ufs_vdisk_new failure", __func__);
            ufs_sys_term();
            return NULL;
        }

        array_add(&sys->vdisks, vdisk);

        if (!sys->fsroot) {
            /* the first disk we mount *is* the file system root */
            sys->fsroot = ufs_inode_get(vdisk, UFS_ROOT_INODE);
            if (!sys->fsroot) {
                ufs_panic("%s ufs_inode_get failure", __func__);
                ufs_sys_term();
                return NULL;
            }
            
            /* we mount the fsroot by pointing fsroot to itself */
            sys->fsroot->mounted_vdisk = vdisk;
            vdisk->mounted_minode = sys->fsroot;

            /* put fsroot in the mointpoint array */
            array_add(&sys->mountpoint, sys->fsroot);
        }
    }

	/* attempt to mount disk using /etc/fstab */
    if (array_count(&sys->disks) > 1) {
		UFS *ufs = ufsnew();
		if (ufs) {
			/* ufs_fstab() will process the "/etc/fstab" file */
			ufs_fstab(ufs, "/etc/fstab");
		}
		if (ufs) ufsfree(&ufs);
	}

quit:
    return sys;
}

__asm__("\n&FUNC    SETC 'find_dd'");
static TIOTDD *find_dd(const char *ddname)
{
    TIOT        *tiot   = get_tiot();       /* tiot for this TCB        */
    UINT32      next    = 0;
    TIOTDD      *tiotdd = (TIOTDD*)tiot->TIOTDD;

    for( ; tiotdd->TIOELNGH; next+= tiotdd->TIOELNGH, tiotdd = (TIOTDD*)&tiot->TIOTDD[next]) {
        /* wtodumpf(tiotdd, tiotdd->TIOELNGH, "TIOTDD"); */
        if (memcmp(ddname, tiotdd->TIOEDDNM, 8)==0) return tiotdd;
    }

    return NULL;
}
