UFS Change Log November 2024

    Addition modifications to support 64 bit time values for
    the disk update_time and create_time fields.

    These fields are in the disk super block (1) in previous 
    versions of UFS. The fields (32 bit) will continue  to be 
    in the super block and are not set to zero.

    New update_time and create_time fields (64 bit) will be
    stored in the boot block (0) extension.

    The boot block and the new extension area will be updated
    whenever the super block is updated and flushed to disk.

