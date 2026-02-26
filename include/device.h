#ifndef DEVICE_H
#define DEVICE_H
/* Borrowed from XINU project with tweaks for MVS38J and CRENT370 */

/* device.h - isbaddev */

#define	DEVNAMLEN	16	/* maximum size of a device name	*/

/* Macro used to verify device ID is valid  */

#define isbaddev(f)  ( ((f) < 0) | ((f) >= NDEVS) )

#endif /* DEVICE_H */
