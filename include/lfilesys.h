#ifndef LFILESYS_H
#define LFILESYS_H
/* Borrowed from XINU project with tweaks for MVS38J and CRENT370 */

/* Notes
**  File names exist within a directory entry. A directory entry
**  associates index blocks with a given file.  That is a directory
**  entry points to index blocks, not file data itself.
**
**  Index blocks point to one or more data blocks that hold the
**  contents of a given file or directory.  That is index blocks
**  point to data blocks.
**
**  Index blocks also point to additional index blocks when a file
**  or directory data exceeds what can be addressed by a single
**  index block.
**
**  directories are special kind of file with a fixed record size
**  that must fit evenly within a disk block (sector).
**
**  Once a directory has been opened, each directory entry can be read
**  sequentially, or randomly using the fixed size of the directory
**  entry to calculate which block contains the desired entry.
**
**  Directories have one or more index blocks and data blocks just
**  like a normal file.
**
**  The file system root directory, "/", is located via the disk info
**  from the first block (block 0) which has the index block number
**  for the first (root) directory.
*/


#ifndef	Nlfl
#define	Nlfl	1
#endif

#ifndef	LF_DISK_DEV
#define	LF_DISK_DEV	SYSERR
#endif

#define	LF_BLKSIZ       1024        /* must be power of 2               */

#define LF_AREA_DI      0           /* first sector of disk info        */
#define	LF_AREA_IB      1           /* first sector of i-blocks         */

#define	LF_INULL        (ibid32) -1 /* index block null pointer         */
#define	LF_DNULL        (dbid32) -1 /* data block null pointer          */
#define	LF_IBLEN        16          /* data block ptrs per i-block      */
#define	LF_IDATA        (LF_BLKSIZ*LF_IBLEN) /* bytes of data           */
                                    /* indexed by a single index blk    */
#define	LF_IMASK        (LF_IDATA-1)/* mask for the data indexed by     */
                                    /* a single index block (i.e.,      */
                                    /* bytes 0 through 16383).          */
#define	LF_DMASK        (LF_BLKSIZ-1)/* mask for the data in a data     */
                                    /*  block (0 - block size-1)        */

/* Structure of an index block on disk */
struct lfiblk {                     /* format of index block            */
    ibid32      next;               /* 00 ID of next index block        */
    uint32      offset;             /* 04 first data byte of the file   */
                                    /*    indexed by this i-block (0)   */
    dbid32      dba[LF_IBLEN];      /* 08 ptrs to data blocks           */
};                                  /* 48 (72 bytes)                    */
#define LF_IBSIZE   sizeof(struct lfiblk)   /* size of index block      */
#define LF_IBSBLK   (LF_BLKSIZ / LF_IBSIZE) /* index blks per phys blk  */

/* Conversion between index block number and disk sector number */
#define	ib2sect(ib)	(((ib)/LF_IBSBLK)+LF_AREA_IB)

/* Conversion between index block number and the relative offset within	*/
/*	a disk sector							*/
#define	ib2disp(ib)	(((ib)%LF_IBSBLK)*LF_IBSIZE)

/* Structure used in each directory entry for the local file system     */
#define	LF_NAME_LEN     64          /* length of file/directory name    */
                                    /* *used to round up size of dir*   */
struct ldentry {                    /* description of entry for one	    */
                                    /* file in the directory            */
    uint32      size;               /* 00 curr. size of file in bytes   */
    ibid32      first;              /* 04 ID of first i-block for file  */
                                    /* or directory,                    */
                                    /* or -1 for empty file/dir         */
    time_t      created;            /* 08 creation date                 */
    time_t      modified;           /* 0C last modified date            */
    byte        type;               /* 10 type of directory entry       */
#define LD_TYPE_FREE        0       /* this entry is available          */
#define LD_TYPE_FILE        1       /* ld_name is a file name           */
#define LD_TYPE_DIR         2       /* ld_name is a directory name      */

    byte        owner;              /* 11 owner access bits             */
    byte        group;              /* 12 group access bits             */
    byte        world;              /* 13 everyone else access bits     */
#define LD_ACC_R            0x04    /* ... read access allowed          */
#define LD_ACC_W            0x02    /* ... write access allowed         */
#define LD_ACC_X            0x01    /* ... execute access allowed       */
#define CAN_READ(byte)  ((byte)&LD_ACC_R)
#define CAN_WRITE(byte) ((byte)&LD_ACC_W)
#define CAN_EXEC(byte)  ((byte)&LD_ACC_X)

    char        ownername[8+1];     /* 14 owner userid                  */
    char        groupname[8+1];     /* 1D group name                    */
    char        lastname[8+1];      /* 26 last updated by userid        */
    char        name[LF_NAME_LEN+1];/* 2F null-terminated file name     */

    char        padding[16];        /* 70 make dir size power of 2      */
};                                  /* 80 (128 bytes)                   */
#define LF_DBSIZE   sizeof(struct ldentry)  /* size of directory entry  */

/* Structure of a index/data block on a free list on disk */
struct lfdbfree {
    dbid32      next;               /* next data block on the list      */
    char        padding[LF_BLKSIZ - sizeof(dbid32)];
};

/* Format of the file system directory, either on disk or in memory */
#define LF_DBSBLK   (LF_BLKSIZ/LF_DBSIZE) /* dir entries per block      */
#define	LF_NUM_DIR_ENT  LF_DBSBLK   /* dir entries per block            */
#pragma pack(2)
struct lfdirblk {                   /* directory block on disk          */
    struct ldentry dir[LF_DBSBLK];  /* set of directory entries         */
};
#pragma pack()

/* Format of the file system, either on disk or in memory */
#pragma pack(2)
struct lfdiskinfo {         /* entire disk info on disk	            */
    uint16  version;        /* 00 version number                    */
    uint16  blksize;        /* 02 disk block size                   */
    dbid32  free_dblk;      /* 04 list of free d-blocks on disk     */
    ibid32  free_iblk;      /* 08 list of free i-blocks on disk     */
    ibid32  dir_iblk;       /* 0C directory i-block for "/"         */
    dbid32  total_dblks;    /* 10 total number of data blocks       */
    dbid32  total_iblks;    /* 14 total number of i-blocks          */
    dbid32  total_blocks;   /* 18 total number of blocks on disk    */
    uint32  unused;         /* 1C unused                            */
};                          /* 20 (32 bytes)                        */
#pragma pack()

/* Control block for local file pseudo-device */
struct lflcblk {            /* Local file control block             */
                            /* (one for each open file)             */
    did32   lfs;            /* 00 device ID of file system  (lfs)   */
    did32   lfl;            /* 04 device ID of this device  (lfl)   */

    byte    mode;           /* 08 mode (read/write/both)            */
#define	LF_MODE_R       F_MODE_R    /* mode bit for "read"          */
#define	LF_MODE_W       F_MODE_W    /* mode bit for "write"         */
#define	LF_MODE_RW      F_MODE_RW   /* mode bits for "read or write"*/
#define	LF_MODE_O       F_MODE_O    /* mode bit for "old"           */
#define	LF_MODE_N       F_MODE_N    /* mode bit for "new"           */
    bool8   ib_dirty;       /* 09 Has iblock changed?               */
    bool8   db_dirty;       /* 0A Has dblock changed?               */
    bool8   dir_dirty;      /* 0B Has dir changed?                  */

    char    *path;          /* 0C strdup() of path to this file     */
    ibid32  dir_iblk;       /* 10 first i-block for this directory  */
    struct ldentry dir;     /* 14 copy of dir entry (128 bytes)     */

    ibid32  inum;           /* 94 ID of current index block in      */
                            /*    iblock or LF_INULL                */
    struct lfiblk iblock;   /* 98 In-mem copy of current index      */
                            /*    block                             */

    dbid32  dnum;           /* E0 Number of current data block      */
                            /*    in dblock or LF_DNULL             */
    uint32  pos;            /* E4 Byte position of next byte to     */
                            /*    read or write                     */
    char  dblock[LF_BLKSIZ];/* E8 in-mem copy of current data block */
};                          /* 4E8 (1256 bytes)                     */

#endif /* LFILESYS_H */
