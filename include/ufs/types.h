#ifndef UFS_TYPES_H
#define UFS_TYPES_H

#ifndef TYPE_BYTE
#define TYPE_BYTE
typedef	unsigned char   BYTE;
#endif

#ifndef TYPE_INT32
#define TYPE_INT32
typedef	int             INT32;
#endif

#ifndef TYPE_INT16
#define TYPE_INT16
typedef	short           INT16;
#endif

#ifndef TYPE_UINT32
#define TYPE_UINT32
typedef	unsigned int    UINT32;
#endif

#ifndef TYPE_UINT16
#define TYPE_UINT16
typedef	unsigned short  UINT16;
#endif

#ifndef TYPE_BOOL
#define TYPE_BOOL
typedef INT32           BOOL;
#endif

#if 0 /* deprecated */
#ifndef TYPE_DSECS
#define TYPE_DSECS
typedef double          DSECS;
#endif
#endif

#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif /* MIN */

#ifndef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif /* MAX */

/* all of our structure typedef's in one spot */
typedef struct ufs_disk         UFSDISK;    /* physical disk dataset handle     */
typedef struct ufs_boot         UFSBOOT;    /* boot block (disk info)           */
typedef struct ufs_boot_ext     UFSBOOTE;   /* boot block extension             */
typedef struct ufs_superblock   UFSSB;      /* disk/memory super block          */

typedef struct ufs_dinode       UFSDIN;     /* disk inode                       */
typedef struct ufs_minode       UFSMIN;     /* memory inode                     */
typedef struct ufs_freeblock    UFSFB;      /* free block                       */

typedef struct ufsvdisk         UFSVDISK;   /* virtual disk                     */

typedef struct ufs_dev          UFSDEV;     /* device handle                    */
typedef struct ufs_devio        UFSDEVIO;   /* device io vector                 */

typedef struct ufsio            UFSIO;      /* i/o vector                       */

typedef struct ufs_dirent       UFSDIR;     /* directory entry                  */
typedef struct ufs_lookup_arg   UFSLKARG;   /* lookup arg                       */
typedef struct ufs_dirlist      UFSDLIST;   /* directory list record            */
typedef struct ufs_dirdesc      UFSDDESC;   /* directory descriptor             */

typedef struct ufs_fileattr     UFSATTR;    /* file attributes                  */

typedef struct ufs_cwd          UFSCWD;     /* current working directory        */

typedef struct ufs_file         UFSFILE;    /* opened file descriptor           */

#endif /* UFS_TYPES_H */
