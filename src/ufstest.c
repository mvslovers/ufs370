#include "ufs/name.h"
#include "ufs/sys.h"
#include "ufs/panic.h"
#include "ufs.h"

int main(int argc, char **argv)
{
    UFSSYS  *sys = ufs_sys_new();
    UFS     *ufs;
    UFSDEV  *namedev;
    UFSNAME **names;
    UFSDISK *disk;
    UFSCWD  *cwd;
    UINT32  count;
    UINT32  n;
    INT32   rc;
    UFSDDESC *desc;
    UFSDLIST *list;
    UFSMIN	*dir_minode	= NULL;
    UFSMIN	*last_minode = NULL;
    char	*path = NULL;

    count = array_count(&sys->disks);
    for(n=0; n < count; n++) {
        disk = sys->disks[n];
        if (!disk) continue;
        wtof("Disk #%u DD=\"%s\", DSN=\"%s\"", n, disk->ddname, disk->dsname);
    }

    ufs = ufsnew();

    wtof("%s ufsnew()", __func__);
    wtof("%s ufs=%08X", __func__, ufs);

    wtof("========");

#if 1
	path = "UFSDISK1";
	rc = ufs_lookup_minode(ufs, path, &dir_minode, &last_minode, NULL);
	wtof("%s: ufs_lookup_minode(\"%s\") rc=%d %s", __func__, path, rc, rc ? strerror(rc) : "SUCCESS");
	wtof("%s:    dir minode=%08X", __func__, dir_minode);
	wtof("%s:    last minode=%08X", __func__, last_minode);

    wtof("========");
#endif

#if 0
    wtof("%s ufs_chgdir(\"%s\")", __func__, "/this/is/a/test");
    rc = ufs_chgdir(ufs, "/this/is/a/test");
    if (rc) wtof("%s rc=%d, %s", __func__, rc, strerror(rc));

    wtof("========");

    wtof("%s ufs_mkdir(\"%s\")", __func__, "this");
    rc = ufs_mkdir(ufs, "this");
    if (rc) wtof("%s rc=%d, %s", __func__, rc, strerror(rc));

    wtof("========");

    wtof("%s ufs_chgdir(\"%s\")", __func__, "this");
    rc = ufs_chgdir(ufs, "this");
    if (rc) wtof("%s rc=%d, %s", __func__, rc, strerror(rc));

    wtof("========");
#endif

#if 1
    desc = ufs_diropen(ufs, "/UFSDISK1", NULL);
    if (desc) {
        wtof("Directory for \"%s\"", desc->path);
        wtof("Permission Owner    Group    Inode Name");
        for(list=ufs_dirread(desc); list; list=ufs_dirread(desc)) {
            wtof("%s %-8.8s %-8.8s %5u %s", list->attr, list->owner, list->group, list->inode_number, list->name);
        }
        ufs_dirclose(&desc);
    }
    wtof("========");
#endif

#if 0
    ufs_name_mount("/", "", 0);
    ufs_name_mount("/pluto", "", 0);
    ufs_name_mount("/october", "", 0);
    ufs_name_mount("/aaaaaaa", "", 0);
    ufs_name_mount("/abcdefg", "", 0);
    ufs_name_mount("/july", "", 0);

    if (sys->names) {
        names = sys->names;
        count = arraycount(&names);
        for(n=0; n < count; n++) {
            wtof("\"%s\"", names[n]->prefix);
        }
    }
#endif
    ufs_sys_term();
    return 0;
}
